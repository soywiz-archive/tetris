#ifndef __TETRIS_MENU_H
#define __TETRIS_MENU_H

	int action_clear_options();
	int action_add_option(int value, Image image);
	int action_input_text(char *title, char *text, int mlength);
	int action_select_option(int currentoption, int escape);

#endif
