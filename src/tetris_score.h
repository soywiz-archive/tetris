#ifndef __TETRIS_SCORE_H
#define __TETRIS_SCORE_H

	#define MAX_SCORES 6

	extern int  score_points[MAX_SCORES];
	extern char score_names[16][MAX_SCORES];
	void score_save();
	void score_load();
	int score_better(int v);
	void score_submit(char *name, int points);

#endif
