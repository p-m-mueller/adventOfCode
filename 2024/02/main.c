#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

const unsigned int MAX_LINE_LENGTH = 1024;
const unsigned int MAX_REPORTS = 1E4;

#define REPORT_SIZE 5

typedef unsigned int ErrorCode;

#define Call(ret) \
  { \
    if (ret != EXIT_SUCCESS) \
      fprintf(stderr, "Error in line %d.\n", __LINE__); \
  }

typedef struct Reports_p *Reports;

struct Reports_p
{
  unsigned int n_reports;
  size_t *report_start_p;
  unsigned int *reports; 
  _Bool allocated;
};

ErrorCode
Reports_destroy(Reports *reports)
{
  if ( (*reports)->allocated )
  {
    free( (*reports)->reports );
  }
  free( (*reports) );

  return EXIT_SUCCESS;
}

ErrorCode
Reports_create(Reports *reports)
{
  (*reports) = NULL;
  (*reports) = (struct Reports_p *)malloc( sizeof(struct Reports_p) );
  if ( *reports == NULL )
  {
    perror("Failed to allocate Reports.");
    return EXIT_FAILURE;
  }

  (*reports)->allocated = false;
  (*reports)->reports = NULL;
  (*reports)->n_reports = 0;

  return EXIT_SUCCESS;
}

ErrorCode
Reports_read_from_file(Reports reports, const char *filename, const unsigned int char_len)
{
  assert(strlen(filename) == char_len);

  FILE *fp = fopen(filename, "r");

  if (fp == NULL)
  { 
    fprintf(stderr, "Failed to open file %s.\n", filename);
    return EXIT_FAILURE;
  }
  else 
  {
    printf("Reading reports from file %s\n", filename);
    char buf[MAX_LINE_LENGTH];

    // Phase 1: Count number of reports
    reports->n_reports = 0;
    while (fgets(buf, MAX_LINE_LENGTH, fp) != NULL)
    {
      reports->n_reports++;

      // Prevent runaway code
      if (reports->n_reports > MAX_REPORTS)
      {
        printf("To many reports in file. File exceeds %d lines.\n", MAX_REPORTS);
        return EXIT_FAILURE;
      }
    }

    printf("Number of reports: %d\n", reports->n_reports);

    reports->report_start_p = (size_t *)malloc( (reports->n_reports + 1) * sizeof(size_t) );

    rewind(fp);

    // Phase 2: Get report sizes and offsets
    reports->report_start_p[0] = 0;
    for (size_t i = 0; i < reports->n_reports; ++i)
    {
      if (fgets(buf, MAX_LINE_LENGTH, fp) != NULL)
      {
        size_t offset = 0;
        char *level_entry = strtok(buf, " ");
        while (level_entry != NULL)
        {
          offset++;
          level_entry = strtok(NULL, " ");
        }

        reports->report_start_p[i+1] = reports->report_start_p[i] + offset;
      }
    }

    reports->reports = (unsigned int *)malloc( reports->report_start_p[reports->n_reports] * sizeof(unsigned int) );
    reports->allocated = true;

    rewind(fp);

    // Phase 3: Get report entries
    for (size_t i = 0; i < reports->n_reports; ++i)
    {
      if (fgets(buf, MAX_LINE_LENGTH, fp) != NULL)
      {
        char *level_entry = strtok(buf, " ");
        for (size_t j = reports->report_start_p[i]; j < reports->report_start_p[i+1]; ++j)
        {
          sscanf(level_entry, "%d", &reports->reports[j]);
          level_entry = strtok(NULL, " "); 
        }
      }
    }

    fclose(fp);
  }


  return EXIT_SUCCESS;
}

size_t
Reports_safe_reports_count(Reports reports)
{
  size_t count = 0;

  for (size_t i = 0; i < reports->n_reports; ++i)
  {

    _Bool is_safe = true;
    _Bool all_levels_increasing = true;
    _Bool all_levels_decreasing = true;

    for (unsigned int j = reports->report_start_p[i]; j < reports->report_start_p[i+1]-1; ++j)
    {
      int diff = reports->reports[j] - reports->reports[j+1];

      if (diff < 0)
      {
        all_levels_increasing = false;
      }
      
      if (diff > 0)
      {
        all_levels_decreasing = false;
      }

      if (!all_levels_increasing && !all_levels_decreasing)
      {
        is_safe = false;
        break;
      }

      if (abs(diff) < 1 || abs(diff) > 3)
      {
        is_safe = false;
        break;
      }
    }

    if (is_safe)
    {
      count++;
    }

  }

  return count;
}


int
main(int argc, char **argv)
{

  if (argc < 2)
  {
    fprintf(stderr, "Not enough input parameters. Usage: %s \"inputfile\"", argv[0]);
    return EXIT_FAILURE;
  }
  else 
  {
    Reports reports = NULL;

    Call(Reports_create(&reports)); 

    Call(Reports_read_from_file(reports, argv[1], strlen(argv[1])));

    printf("Part 1: Number of safe reports: %zu\n", Reports_safe_reports_count(reports));

    Call(Reports_destroy(&reports)); 
  }

  return EXIT_SUCCESS;
}
