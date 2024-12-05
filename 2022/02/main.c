#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define MAX_LINE_LENGTH 16

// Part 1 data
enum OpponentShape {ROCKo = 'A', PAPERo = 'B', SCISSORSo = 'C'};
enum PlayerShape {ROCKp = 'X', PAPERp = 'Y', SCISSORSp = 'Z'};

// Part 2 data
enum Outcome {LOSE = 'X',  DRAW = 'Y', WIN = 'Z'};

typedef struct Game_p *Game;

struct Game_p
{
	int rounds;
	char *shape_1;
	char *shape_2;
	char *outcome;
};

void Game_create(Game *game)
{
	*game = (struct Game_p*)malloc( sizeof(struct Game_p) );
}

void Game_destroy(Game *game)
{
	free((*game)->shape_1);
	free((*game)->shape_2);	
	(*game)->outcome = NULL;
	free(*game);
}

void Game_read_from_file(const char *filename, Game game)
{
	FILE *fp = NULL;
	char buf[MAX_LINE_LENGTH];
	int round;

	game->rounds = 0;

	fp = fopen(filename, "r");
	if (fp == NULL)
		perror("Failed to open file\n");
	else 
	{
		printf("Reading game from %s\n", filename);
		while (fgets(buf, MAX_LINE_LENGTH, fp) != NULL)
			game->rounds++;

		rewind(fp);

		game->shape_1 = (char*)malloc( game->rounds * sizeof(char) );
		game->shape_2 = (char*)malloc( game->rounds * sizeof(char) );

		round = 0;
		while (fgets(buf, MAX_LINE_LENGTH, fp) != NULL)
		{
			sscanf(buf, "%s %s", &game->shape_1[round], &game->shape_2[round]);	
			round++;
		}
	}

	game->outcome = &game->shape_2[0];

	fclose(fp);
}

int Game_outcome_score(const enum OpponentShape o, const enum PlayerShape s)
{
	switch(o)
	{
	case(ROCKo):
		switch(s)
		{
		case(ROCKp):     return 3;
		case(PAPERp):    return 6;
		case(SCISSORSp): return 0;
		default: perror("Unknown player shape.");
		}
	case(PAPERo):
		switch(s)
		{
		case(ROCKp):     return 0;
		case(PAPERp):    return 3;
		case(SCISSORSp): return 6;
		default: perror("Unknown player shape.");
		}
	case(SCISSORSo):
		switch(s)
		{
		case(ROCKp):     return 6;
		case(PAPERp):    return 0;
		case(SCISSORSp): return 3;
		default: perror("Unknown player shape.");
		}
	default: perror("Unknown opponent shape.");
	}
}

int Game_shape_score(const enum PlayerShape p)
{
	switch(p)
	{
	case(ROCKp):     return 1;
	case(PAPERp):    return 2;
	case(SCISSORSp): return 3;
	}
}

int Game_get_outcome_score(const enum OpponentShape o, const enum PlayerShape p)
{
	return Game_outcome_score(o, p) + Game_shape_score(p);
}

int Game_total_shape_score(Game game)
{
	int r, score, sum;

	sum = 0;
	for (r = 0; r < game->rounds; ++r)
	{
		score = Game_get_outcome_score(game->shape_1[r], game->shape_2[r]);
		printf("Game %5d score %2d\n", r, score);
		assert(score > 0);
		sum += score;	
	}
	return sum;	
}

enum PlayerShape Game_player_shape(const enum OpponentShape o, const enum Outcome result)
{
	switch(o)
	{
	case(ROCKo):
		switch(result)
		{
		case(LOSE): return SCISSORSp;
		case(DRAW): return ROCKp;
		case(WIN):  return PAPERp;
		default: perror("Unknown player outcome");
		}
	case(PAPERo):
		switch(result)
		{
		case(LOSE): return ROCKp;
		case(DRAW): return PAPERp;
		case(WIN):  return SCISSORSp;
		default: perror("Unknown player outcome");
		}
	case(SCISSORSo):
		switch(result)
		{
		case(LOSE): return PAPERp;
		case(DRAW): return SCISSORSp;
		case(WIN):  return ROCKp;
		default: perror("Unknown player outcome");
		}
	default: perror("Unknown opponent shape.");
	}
}

int Game_total_outcome_score(Game game)
{
	int r, score, sum;
	enum PlayerShape player_shape;

	sum = 0;
	for (r = 0; r < game->rounds; ++r)
	{
		player_shape = Game_player_shape(game->shape_1[r], game->outcome[r]);
		score =  Game_get_outcome_score(game->shape_1[r], player_shape);
		printf("Game %5d score %2d\n", r, score);
		assert(score > 0);
		sum += score;	
	}
	return sum;
}

int main(int argc, char **argv)
{
	int total_game_score;
	Game game;

	Game_create(&game);
	
	if (argc < 2)
	{
		perror("Not enough arguments.");
		return 1;
	}
	else 
	{
		Game_read_from_file(argv[1], game);

		/* Part 1
		 * Get the total score for all rounds if the second 
		 * key of the encrypted list is the shape of the player.
		 */
		total_game_score = Game_total_shape_score(game);
		printf("Total score of game: %d\n", total_game_score);

		/* Part 2
		 * Get the total score of all rounds if the second
		 * key of the encrypted list is the outcome of the round.
		 */
		total_game_score = Game_total_outcome_score(game);
		printf("Total score by outcome: %d\n", total_game_score);

	}

	Game_destroy(&game);

	return 0;
}
