#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 256

typedef struct Luggage_t *Luggage;

struct Luggage_t
{
	int rucksacks;
	int *rucksack_item_ptr;
	char *items;
};

void Luggage_create(Luggage *luggage)
{
	(*luggage) = NULL;
	(*luggage) = (struct Luggage_t*)malloc( sizeof(struct Luggage_t) );
	if (*luggage == NULL)
		perror("Failed to create luggage\n");
}

void Luggage_destroy(Luggage *luggage)
{
	free((*luggage)->items);
	free((*luggage)->rucksack_item_ptr);
	free(*luggage);
}

void Luggage_read_from_file(const char *filename, Luggage luggage)
{
	FILE *fp = NULL;
	char buf[MAX_LINE_LENGTH];
	size_t len;
	int start, r;

	fp = fopen(filename, "r");
	if (fp == NULL)
		fprintf(stderr, "Failed to open %s\n", filename);
	else
	{
		luggage->rucksacks = 0;
		while(fgets(buf, MAX_LINE_LENGTH, fp) != NULL)
			luggage->rucksacks++;

		printf("Number of rucksacks: %d\n", luggage->rucksacks);

		luggage->rucksack_item_ptr = (int*) malloc( (luggage->rucksacks+1) * sizeof(int) );

		rewind(fp);

		r = 0;
		luggage->rucksack_item_ptr[0] = 0;
		while(fgets(buf, MAX_LINE_LENGTH, fp) != NULL)
		{
			len = strlen(buf)-1;
			luggage->rucksack_item_ptr[r+1] = luggage->rucksack_item_ptr[r] + len;
			r++;
		}

		printf("Number of items: %d\n", luggage->rucksack_item_ptr[luggage->rucksacks]);

		luggage->items = (char*)malloc( luggage->rucksack_item_ptr[luggage->rucksacks] * sizeof(char) );
		
		rewind(fp);

		r = 0;
		while(fgets(buf, MAX_LINE_LENGTH, fp) != NULL)
		{
			start = luggage->rucksack_item_ptr[r];
			len = luggage->rucksack_item_ptr[r+1] - start;
			strncpy(&luggage->items[start], buf, len);
			r++;
		}
	
		fclose(fp);
	}
}

char Luggage_rucksack_wrong_item(const int size, const char *items)
{	
	const char *item1, *item2;
	int i, j, half_idx;

	half_idx = (int)size/2;
	if (size % half_idx != 0) 
		perror("Rucksack contains an odd-number of items.\n");

	for (i = 0; i < half_idx; ++i)
	{
		item1 = &items[i];
		for (j = half_idx; j < size; ++j)
		{
			item2 = &items[j];
			if (*item1 == *item2) return *item2;
		}
	}
}

int Luggage_rucksack_wrong_item_priority(const char *item)
{
	if (!isalpha(*item))
	{
		perror("Character is not alphabetic.\n");
		return 0;
	}

	if (islower(*item))
		return (int)(*item) - 97 + 1;
	else
		return (int)(*item) - 65 + 27;
}

int Luggage_sum_priority_wrong_items(Luggage luggage)
{
	int sum, r, start, len;
	char item;
	int priority;

	sum  = 0;
	for (r = 0; r < luggage->rucksacks; ++r)
	{
		start = luggage->rucksack_item_ptr[r];
		len = luggage->rucksack_item_ptr[r+1] - start;
		item = Luggage_rucksack_wrong_item(len, &luggage->items[start]);
		priority = Luggage_rucksack_wrong_item_priority(&item);
		sum += priority;
	}
	return sum;
}

int main(int argc, char **argv)
{
	Luggage luggage;
	int sum_priority_wrong_itmes;

	Luggage_create(&luggage);

	if (argc < 2)
		perror("Not enough input arguments.\n");
	else 
	{
		Luggage_read_from_file(argv[1], luggage);

		sum_priority_wrong_itmes = Luggage_sum_priority_wrong_items(luggage);
		printf("The sum of the priority of wrong items: %d\n", sum_priority_wrong_itmes);	
	
	}

	Luggage_destroy(&luggage);

	return 0;
}
