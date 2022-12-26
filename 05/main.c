#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>

#define MAX_LINE_LENGTH 256

int is_blank_line(const char *line)
{
	const char *ch;

	for (ch = line; *ch != '\0'; ++ch)
		if (!isspace(*ch)) return 0;
	return 1;
}

typedef struct Supplies_p *Supplies;

struct Supplies_p
{
	unsigned int stacks;
	unsigned int *crates_ptr;	
	char *crates;
	unsigned int rearrangements;
	unsigned int *moves;
};

void Supplies_create(Supplies *supplies)
{
	*supplies = NULL;
	*supplies = (struct Supplies_p*)malloc( sizeof(struct Supplies_p) );
	if (*supplies == NULL)
		perror("Failed to create Supplies.\n");
}

void Supplies_destroy(Supplies *supplies)
{
	free((*supplies)->moves);
	free((*supplies)->crates);
	free((*supplies)->crates_ptr);
	free(*supplies);
	*supplies = NULL;
}

void Supplies_read_from_file(const char *filename, Supplies supplies)
{
	FILE *fp = NULL;
	char buf[MAX_LINE_LENGTH];
	unsigned int s, c, pos, m, stack_size, max_stack_size;
	char *ch;
	regex_t reg;

	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "Failed to open %d.\n", filename);
		return;
	}
	else
	{
		printf("Reading data from \"%s\"\n", filename);

		// Count number of lines for stack pattern
		pos = 0;
		while(fgets(buf, MAX_LINE_LENGTH, fp) != NULL)
			if (!is_blank_line(buf)) 
				pos++;
			else
				break;

		rewind(fp);

		max_stack_size = pos-1;
		printf("Max stack size: %d\n", max_stack_size);

		for (s = 0; s < pos-1; ++s)
			fgets(buf, MAX_LINE_LENGTH, fp);
		
		memset(buf, '\0', MAX_LINE_LENGTH);	
		fgets(buf, MAX_LINE_LENGTH, fp);
		
		supplies->stacks = 0;
		for (ch = buf; *ch != '\0'; ++ch)
			if (!(*ch == ' '))
				supplies->stacks++;

		supplies->stacks--;
		printf("Cargo stacks: %d\n", supplies->stacks);

		supplies->crates_ptr = (unsigned int*)malloc( (supplies->stacks+1) * sizeof(unsigned int) );

		rewind(fp);

		supplies->crates_ptr[0] = 0;
		for (s = 0; s < supplies->stacks; ++s)
		{
			stack_size = 0;
			for (pos = 0; pos < max_stack_size; ++pos)
			{
				fgets(buf, MAX_LINE_LENGTH, fp);
				ch = buf+4*s+1;
				if (*ch != ' ')
					stack_size++;
			}
			supplies->crates_ptr[s+1] = supplies->crates_ptr[s] + stack_size;
			rewind(fp);
		}

		printf("Total number of crates: %d\n", supplies->crates_ptr[supplies->stacks]);

		supplies->crates= (char *)malloc( supplies->crates_ptr[supplies->stacks] * sizeof(char) );

		c = 0;
		for (s = 0; s < supplies->stacks; ++s)
		{
			for (pos = 0; pos < max_stack_size; ++pos)
			{
				fgets(buf, MAX_LINE_LENGTH, fp);
				ch = buf+4*s+1;
				if (*ch != ' ')
				{
					supplies->crates[c] = *ch;
					c++;
				}
			}
			rewind(fp);
		}

		supplies->rearrangements = 0;
		regcomp(&reg, "^move ([0-9]*) from ([0-9]*) to ([0-9]*)", REG_EXTENDED);
		while (fgets(buf, MAX_LINE_LENGTH, fp) != NULL)
		{
			regmatch_t *pmatch;

			if (regexec(&reg, buf, 0, pmatch, 0) == 0)
				supplies->rearrangements++;
		}

		printf("Total number of rearrangements: %d\n", supplies->rearrangements);

		supplies->moves = (unsigned int*)malloc( supplies->rearrangements * 3 * sizeof(unsigned int) );
			
		rewind(fp);

		m = 0;
		while (fgets(buf, MAX_LINE_LENGTH, fp) != NULL)
		{
			regmatch_t *pmatch;

			if (regexec(&reg, buf, 0, pmatch, 0) == 0)
			{
				sscanf(buf, "move %d from %d to %d", &supplies->moves[m*3+0], &supplies->moves[m*3+1], &supplies->moves[m*3+2]);
				
				supplies->moves[m*3+1]--;
				supplies->moves[m*3+2]--;
				
				m++;
			}
		}
		regfree(&reg);
	}
	fclose(fp);
}


/*
 *  p
 *  5 |   |   |   |
 *  4 |   |   |   |
 *  3 |   |   |   |
 *  2 |   | D |   |
 *  1 | N | C |   |
 *  0 | Z | M | P |
 *     0   1   2    s
 *
 */
void Supplies_apply_moves(Supplies supplies)
{
	unsigned int *stack_fill;
	char *crates_map;
	unsigned int max_stack_size = supplies->crates_ptr[supplies->stacks];
	unsigned int s, p, r, m, c, number_of_crates, source_stack, dest_stack, source_idx, dest_idx;

	stack_fill = (unsigned int*)malloc( supplies->stacks * sizeof(unsigned int) );
	crates_map = (char*)malloc( supplies->stacks * max_stack_size * sizeof(char) );

	// Copy to fixed size data structure
	for (s = 0; s < supplies->stacks; ++s)
	{
		for (p = 0; p < max_stack_size; ++p)
			crates_map[s*max_stack_size+p] = ' ';

		for (stack_fill[s] = 0, p = supplies->crates_ptr[s+1]; p > supplies->crates_ptr[s]; --p, stack_fill[s]++)
			crates_map[s*max_stack_size+stack_fill[s]] = supplies->crates[p-1];
	}

	printf("Initial configuration:\n");
	for (s = 0; s < supplies->stacks; ++s)
	{
		printf("%d : [ ", s);
		for (p = 0; p < max_stack_size; ++p)
			printf("%c ", crates_map[s*max_stack_size+p]);
		printf("] (%d)\n", stack_fill[s]);
	}

	// Move crates
	for (r = 0; r < supplies->rearrangements; ++r)
	{
		number_of_crates = supplies->moves[r*3+0];
		source_stack = supplies->moves[r*3+1];
		dest_stack = supplies->moves[r*3+2];

		for (m = 0; m < number_of_crates; ++m)
		{
			source_idx = source_stack*max_stack_size+stack_fill[source_stack]-1;
			dest_idx = dest_stack*max_stack_size+stack_fill[dest_stack];

			crates_map[dest_idx] = crates_map[source_idx];
			crates_map[source_idx] = ' ';
			stack_fill[dest_stack]++;
			stack_fill[source_stack]--;
		}
	}

	printf("Final configuration:\n");
	for (s = 0; s < supplies->stacks; ++s)
	{
		printf("%d : [ ", s);
		for (p = 0; p < max_stack_size; ++p)
			printf("%c ", crates_map[s*max_stack_size+p]);
		printf("] (%d)\n", stack_fill[s]);
	}

	// Copy back to compressed data structure
	supplies->crates_ptr[0] = 0;
	c = 0;
	for (s = 0; s < supplies->stacks; ++s)
	{
		supplies->crates_ptr[s+1] = supplies->crates_ptr[s] + stack_fill[s];

		for (p = 0; p < stack_fill[s]; ++p)
		{
			supplies->crates[c] = crates_map[s*max_stack_size+p];
			c++;
		}
	}


	free(crates_map);
	free(stack_fill);	
}

int main(int argc, char **argv)
{
	Supplies supplies;

	if (argc < 2)
	{
		perror("Not enough input arguments.\n");
		return 1;
	}
	else
	{
		Supplies_create(&supplies);

		Supplies_read_from_file(argv[1], supplies);

		Supplies_apply_moves(supplies);

		printf("Upper most crates in stacks:\n");
		printf("Stack: ");
		for (int s = 0; s < supplies->stacks; ++s)
			printf("%d ", s);
		printf("\n");
		printf("Crate: ");
		for (int s = 0; s < supplies->stacks; ++s)
			printf("%c ", supplies->crates[supplies->crates_ptr[s+1]-1]);
		printf("\n");

		Supplies_destroy(&supplies);
	}

	return 0;
}
