#include <stdlib.h>
#include <stdio.h>

#define MAX_LINE_LENGTH 256

typedef struct Campdb_p *Campdb;

struct Campdb_p
{
	unsigned int pairs;
	unsigned int *section_ranges;
};

void Campdb_create(Campdb *cdb)
{
	(*cdb) = NULL;
	(*cdb) = (struct Campdb_p*)malloc( sizeof(struct Campdb_p) );
	if (*cdb == NULL)
		perror("Failed to create Campdb");
}

void Campdb_destroy(Campdb *cdb)
{
	free((*cdb)->section_ranges);
	free(*cdb);
}

void Campdb_read_from_file(const char *filename, Campdb cdb)
{
	FILE *fp = NULL;
	char buf[MAX_LINE_LENGTH];
	unsigned int p;
	unsigned int *ranges;

	fp = fopen(filename, "r");
	if (fp == NULL)
		fprintf(stderr, "Failed to open file %d\n", filename);
	else
	{
		cdb->pairs = 0;
		while(fgets(buf, MAX_LINE_LENGTH, fp) != NULL)
			cdb->pairs++;
	
		printf("%d pairs found\n", cdb->pairs);	

		cdb->section_ranges = NULL;
		cdb->section_ranges = (unsigned int*)malloc( cdb->pairs * 4 * sizeof(unsigned int) );

		rewind(fp);

		p = 0;
		while(fgets(buf, MAX_LINE_LENGTH, fp) != NULL)
		{
			ranges = &cdb->section_ranges[p*4];
			sscanf(buf, "%d-%d,%d-%d", &ranges[0], &ranges[1], &ranges[2], &ranges[3]);
			p++;
		}
	}
	fclose(fp);
}

/* Test if interval I0 = [s0, e0] is fully contained in I1 = [s1, e1]. This is the case if
 *
 *  s1 <= s0 <= e0 <= e1
 */
unsigned int Pair_contained(const unsigned int *restrict I0, const unsigned int *restrict I1)
{
	if ( I1[0] <= I0[0] && I0[1] <= I1[1] ) 
		return 1;
	else 
		return 0;
}


unsigned int Campdb_sum_contained_ranges(Campdb cdb)
{
	unsigned int sum, p, *ranges;

	sum = 0;
	for (p = 0; p < cdb->pairs; ++p)
	{
		ranges = &cdb->section_ranges[p*4];
		if (Pair_contained(ranges, ranges+2))
			sum++;
		else if (Pair_contained(ranges+2, ranges))
			sum++;
	}
	return sum;
}


int main(int argc, char **argv)
{
	Campdb cdb;
	unsigned int sum_camp_ranges_containted;

	Campdb_create(&cdb);

	if (argc  < 2)
		perror("Not enough input arguments.\n");
	else
	{
		Campdb_read_from_file(argv[1], cdb);

		sum_camp_ranges_containted = Campdb_sum_contained_ranges(cdb);
		printf("Number of section ranges that are fully contained in there peers range: %d\n", sum_camp_ranges_containted);
	}
	Campdb_destroy(&cdb);

	return 0;
}
