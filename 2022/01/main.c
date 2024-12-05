#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#define MAX_LINE_LENGTH 256

int is_blank_line(const char *line)
{
	const char *ch;

	for (ch = line; *ch != '\0'; ++ch)
		if (!isspace(*ch)) return 0;

	return 1;
}

typedef struct Elfdb_p *Elfdb;

struct Elfdb_p
{
	int elfs;
	int total_number_of_items;
	int *food_items_per_elf;
	int *elf_item_ptr;
	int *calories_per_item;
};

void Elfdb_create(Elfdb *edb)
{
	*edb = NULL;
	*edb = (struct Elfdb_p*)malloc( sizeof(struct Elfdb_p) );
	if (*edb == NULL)
		perror("Failed to allocate Elfdb\n");
}

void Elfdb_free(Elfdb *edb)
{
	free((*edb)->calories_per_item);
	free((*edb)->elf_item_ptr);
	free((*edb)->food_items_per_elf);
	free(*edb);
}

void Elfdb_read_from_file(const char *filename, Elfdb edb)
{
	FILE* fp = NULL;
	char buf[MAX_LINE_LENGTH];
	int e, i;

	edb->elfs = 1;
	edb->total_number_of_items = 0;

	fp = fopen(filename, "r");
	if (fp == NULL) 
	{
		perror("Failed to open file.\n");
	}
	else
	{
		printf("Reading Elf input file: %s\n", filename);
		while (fgets(buf, MAX_LINE_LENGTH, fp) != NULL)
		{
			if (!is_blank_line(buf)) 
				edb->total_number_of_items++;
			else
				edb->elfs++;
	
		}

		printf("\tNumber of Elfs: %d\n", edb->elfs);
		printf("\tTotal number of items: %d\n", edb->total_number_of_items);

		edb->food_items_per_elf = (int*)malloc( edb->elfs * sizeof(int) );

		rewind(fp);
			
		e = 0;
		while(fgets(buf, MAX_LINE_LENGTH, fp) != NULL)
		{
			if (!is_blank_line(buf)) 
				edb->food_items_per_elf[e]++;
			else
				e++;
		}


		edb->elf_item_ptr = (int*)malloc( (edb->elfs+1) * sizeof(int) );
		edb->elf_item_ptr[0] = 0;

		for (e = 0; e < edb->elfs; ++e)
			edb->elf_item_ptr[e+1] = edb->elf_item_ptr[e] + edb->food_items_per_elf[e];

		edb->calories_per_item = (int*)malloc( edb->elf_item_ptr[edb->elfs] * sizeof(int) );

		rewind(fp);

		i = 0;
		while(fgets(buf, MAX_LINE_LENGTH, fp) != NULL)
			if (!is_blank_line(buf))
			{
				sscanf(buf, "%d", &edb->calories_per_item[i]);
				i++;
			}
	}

	fclose(fp);
}

void Elfdb_get_elf_max_calories(const Elfdb edb, int *elf_idx, int *calories)
{
	int elf_total_calories;
	int e, i;

	*elf_idx = -1;
	*calories = -1;
	for (e = 0; e < edb->elfs-1; ++e)
	{
		elf_total_calories = 0;
		for (i = edb->elf_item_ptr[e]; i < edb->elf_item_ptr[e+1]; ++i)
			elf_total_calories += edb->calories_per_item[i];

		if (elf_total_calories > *calories) 
		{
			*elf_idx = e;
			*calories = elf_total_calories;
		}
	}
}

void swap(int *l, int *r)
{
	int tmp = *l;
	*l = *r;
	*r = tmp;
}

int partition(int left, int right, int pivot, int *list)
{
	int lp = left-1;
	int rp = right;

	while(1)
	{
		while(list[++lp] < pivot) { }
		while(rp > 0 && list[--rp] > pivot) { }

		if (lp >= rp)
			break;
		else 
			swap(&list[lp], &list[rp]);
	}
	swap(&list[lp], &list[right]);
	return lp;

}

void quick_sort(int left, int right, int *list)
{
	int pivot, part_ptr;

	if (right - left <= 0)
		return;
	else 
	{
		pivot = list[right];
		part_ptr = partition(left, right, pivot, list);
		quick_sort(left, part_ptr-1, list);
		quick_sort(part_ptr+1, right, list);
	}
}

void sort(const int n, int *list)
{
	quick_sort(0, n-1, list);
}

void Elfdb_get_top_three_elf_calories(const Elfdb edb, int *calories)
{
	int *elf_total_calories;
	int e, i, idx;

	elf_total_calories = (int*)malloc( edb->elfs * sizeof(int) );

	for (e = 0; e < edb->elfs; ++e)
	{
		elf_total_calories[e] = 0;
		for (i = edb->elf_item_ptr[e]; i < edb->elf_item_ptr[e+1]; ++i)
			elf_total_calories[e] += edb->calories_per_item[i];
	}

	sort(edb->elfs, elf_total_calories);

	for (e = 0; e < 3; ++e)
	{
		idx = edb->elfs-3+e;
		calories[e] = elf_total_calories[idx];
	}

	free(elf_total_calories);
}

int main(int argc, char **argv)
{
	int elf_with_max_calories, max_calories_of_single_elf;
	int top_three_elf_calories[3], total_top_three_elf_calories;
	int e;

	Elfdb edb;


	if (argc < 2) 
	{
		fprintf(stderr, "Not enough arguments.\n");
		return 1;
	}
	else
	{
		Elfdb_create(&edb);
		Elfdb_read_from_file(argv[1], edb);

		// Part 1
		Elfdb_get_elf_max_calories(edb, &elf_with_max_calories, &max_calories_of_single_elf);

		printf("Elf %d carries the most calories. He carries %d calories.\n", elf_with_max_calories, max_calories_of_single_elf);
	
		// Part 2	
		Elfdb_get_top_three_elf_calories(edb, top_three_elf_calories);
		printf("Top tree Elfs carrying calories\n");
		total_top_three_elf_calories = 0;
		for (e = 0; e < 3; ++e)
		{
			total_top_three_elf_calories += top_three_elf_calories[e];
			printf("Elf %5d carries %8d calories\n", e, top_three_elf_calories[e]);
		}
		printf("The top three Elfs carrie %8d calories in total.\n", total_top_three_elf_calories);

		Elfdb_free(&edb);
	}

	return 0;
}
