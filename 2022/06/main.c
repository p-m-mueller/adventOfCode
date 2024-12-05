#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Elfstream_p *Elfstream;

struct Elfstream_p
{
	unsigned int len;
	char *buf;	
};

void Elfstream_create(Elfstream *es)
{
	*es = NULL;
	*es = (struct Elfstream_p*)malloc( sizeof(struct Elfstream_p) );
	if (*es == NULL)
		perror("Failed to create Elfstream\n");
}

void Elfstream_destroy(Elfstream *es)
{
	free((*es)->buf);
	free(*es);
}

void Elfstream_read_from_file(const char *filename, Elfstream es)
{
	FILE *fp = NULL;

	fp = fopen(filename, "r");
	if (fp == NULL)
		fprintf(stderr, "Failed to oepn file \'%s\'\n",filename);
	else
	{
		printf("Reading datastream buffer from file \'%s\'\n", filename);

		es->len = 0;
		while (fgetc(fp) != EOF) 
			es->len++;

		es->buf = (char*)malloc( es->len * sizeof(char) );

		rewind(fp);

		fgets(es->buf, es->len, fp);

		fclose(fp);
	}
}

int Block_all_char_differ(const unsigned int len, const char *cblock)
{
	int i, j;

	//printf("%s\n", cblock);

	for (i = 0; i < len; ++i)
		for (j = i+1; j < len; ++j)
		{
			//printf(" %c ?= %c\n", cblock[i], cblock[j]);
			if (cblock[i] == cblock[j]) return 0;
		}

	return 1;
}

int Elfstream_get_start_of_pack_marker(Elfstream es)
{
	char cblock[4];
	int c;

	for (c = 4; c < es->len; ++c)
	{
		memcpy(cblock, es->buf+c-4, 4*sizeof(char));
		if (Block_all_char_differ(4, cblock)) 
			return c;
	}
}

int Elfstream_get_start_of_message_marker(Elfstream es)
{
	char cblock[14];
	int c;

	for (c = 14; c < es->len; ++c)
	{
		memcpy(cblock, es->buf+c-14, 14*sizeof(char));
		if (Block_all_char_differ(14, cblock)) 
			return c;
	}
}

int main(int argc, char **argv)
{
	Elfstream es;
	int start_of_pack_marker, start_of_message_marker;

	if (argc < 2)
	{
		perror("Not enough input arguments.\n");
		return 1;
	}
	else 
	{
		Elfstream_create(&es);

		Elfstream_read_from_file(argv[1], es);

		// Part 1
		start_of_pack_marker = Elfstream_get_start_of_pack_marker(es);
		printf("Start of pack marker at: %d\n", start_of_pack_marker);
		
		// Part 2
		start_of_message_marker = Elfstream_get_start_of_message_marker(es);
		printf("Start of message marker at: %d\n", start_of_message_marker);

		Elfstream_destroy(&es);
	}

	return 0;
}
