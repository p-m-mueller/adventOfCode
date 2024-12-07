#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

const size_t MAX_LINE_LENGTH = 32;
const size_t MAX_LOCATIONS = 1E4;

typedef unsigned int ErrorCode;

typedef struct LocationPairList_p *LocationPairList;

struct LocationPairList_p
{
  _Bool allocated;
  unsigned int *locationID1;
  unsigned int *locationID2;
  size_t n_locations;
};

ErrorCode
LocationPairList_create(LocationPairList *ll)
{
  *ll = NULL;
  *ll = (struct LocationPairList_p *)malloc( sizeof(struct LocationPairList_p) );
  if (*ll == NULL)
  {
    perror("Faield to create LocationPairList\n");
    return EXIT_FAILURE;
  }

  // Set default values
  (*ll)->allocated = false;
  (*ll)->locationID1 = NULL;
  (*ll)->locationID2 = NULL;
  (*ll)->n_locations = 0;

  return EXIT_SUCCESS;
}

ErrorCode
LocationPairList_destroy(LocationPairList *ll)
{
  if (*ll != NULL) 
  {
    if ((*ll)->allocated)
    {
      free((*ll)->locationID1);
      free((*ll)->locationID2);
    }

    free((*ll));
  }

  return EXIT_SUCCESS;
}

ErrorCode
LocationPairList_read_from_file(LocationPairList ll, 
                                const char *filename, 
                                const unsigned int length)
{
  // Ensure that the input for the file name has the correct length.
  assert(strlen(filename) == length);

  FILE* fp = fopen(filename, "r");

  char buf[MAX_LINE_LENGTH];

  if (fp == NULL)
  {
    perror("Failed to open file.\n");
    return EXIT_FAILURE;
  }
  else 
  {
    printf("Reading LocationPairList form file %s\n", filename);
      

    // Count numbe rof entries
    ll->n_locations = 0;
    while (fgets(buf, MAX_LINE_LENGTH, fp) != NULL)
    {
        ll->n_locations++;
       
        // Prevent runaway code
        if (ll->n_locations > MAX_LOCATIONS)
        {
          break;
        }
    }

    printf("Locatoin count: %zu\n", ll->n_locations);

    ll->locationID1 = (unsigned int *)malloc( ll->n_locations * sizeof(unsigned int) );
    ll->locationID2 = (unsigned int *)malloc( ll->n_locations * sizeof(unsigned int) );

    ll->allocated = true;

    rewind(fp);
    
    for (unsigned int i = 0; i < ll->n_locations; ++i)
    {
      fgets(buf, MAX_LINE_LENGTH, fp);

      sscanf(buf, "%d %d", &ll->locationID1[i], &ll->locationID2[i]);
    }

    return fclose(fp);
  }
}

/* 
 * I used the non-recursive merge sort algorithm from Rama Hoetzlein anser on
 * https://stackoverflow.com/questions/1557894/non-recursive-merge-sort
 * a similar version can be found on 
 * https://algorithmist.com/wiki/Merge_sort.c
 */
ErrorCode
merge_sort(int *list, size_t size)
{
  int tmp[size];
  unsigned int right, right_end;
  unsigned int i, j, m;

  for (unsigned int k = 1; k < size; k *= 2)
  {
    for (unsigned int left = 0; left + k < size; left += 2 * k)
    {
      right = left + k;
      right_end = right + k;
      if (right_end > size) right_end = size;
      m = i = left;
      j = right;

      while (i < right && j < right_end)
      {
        if (list[i] <= list[j])
        {
          tmp[m] = list[i];
          ++i;
        }
        else 
        {
          tmp[m] = list[j];
          ++j;
        }
        ++m;
      }

      while (i < right)
      {
        tmp[m] = list[i];
        ++i; ++m;
      }

      while (j < right_end)
      {
        tmp[m] = list[j];
        ++j; ++m;
      }

      for (m = left; m < right_end; ++m)
      {
        list[m] = tmp[m];
      }
    }
  }

  return EXIT_SUCCESS;
}


/*
 * The sum of the distacnces between the entries of two list
 * is nothing else than the l1-norm of |u - v| with the two
 * vectors.
 * Here I used the implementation for two integer 
 */
int
l1_error(int *u, int *v, size_t length)
{
  int s = 0;

  for (size_t i = 0; i < length; ++i)
  {
    s += abs(u[i] - v[i]);
  }

  return s;
}

/* To keep things easy here I did not really optimize the algorighm and went 
 * for the naive method. 
 * For each ID in left_list the complete right_list is searched
 * for identical IDs. The score then increases by the ID if the ID occurs in 
 * both lists.
 * The only optimization takes into account that both lists are sorted from
 * low to high IDs and the inner loop over the right IDs stops if the ID from
 * the left ist smaller than the ID from the right.
 *
 * Defenately there is room for improvement.
 */
unsigned int
similarity_socre(unsigned int *left_list, unsigned int *right_list, size_t length)
{
  unsigned int score = 0;

  for (size_t i = 0; i < length; ++i)
  {
    unsigned int left = left_list[i];

    for (size_t j = 0; j < length; ++j)
    {
      unsigned int right = right_list[j];

      if (left == right)
      {
        score += left;
      }
      else if (right > left)
      {
        break;
      }
    }
  }

  return score;
}


int
main(int argc, char **argv)
{
  if (argc < 2)
  {
    perror("Not enough input parameters.\n");
    return EXIT_FAILURE;
  }
  else 
  {
    LocationPairList location_list;

    LocationPairList_create(&location_list);

    LocationPairList_read_from_file(location_list, 
                                    argv[1], 
                                    strlen(argv[1]));

    // Part 1
    merge_sort((int*)location_list->locationID1, 
               location_list->n_locations);

    merge_sort((int*)location_list->locationID2, 
               location_list->n_locations);

    printf("Part 1; Sum of distances: %d\n",
           l1_error((int*)location_list->locationID1, 
                   (int*)location_list->locationID2,
                   location_list->n_locations));
  
    // Part 2
    printf("Part 2; Similarity score: %d\n", 
           similarity_socre(location_list->locationID1, 
                            location_list->locationID2, 
                            location_list->n_locations));

    LocationPairList_destroy(&location_list);
  }


  return EXIT_SUCCESS;
}
