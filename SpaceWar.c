#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <windows.h>
#include "sl.h"
#include "vector.h"

#define LOGARITHMIC_GROWTH
#define WINDOWS_X 1000.0
#define WINDOWS_Y 730.0
#define _255to1(x) x/255.0

typedef struct images
{
	int map[7];
	int jet[7];
	int bullet[7];
	int bot[7];
	int control[2];
	int arrows;
	int cursor;
	int heart;
	int explode;
} images;

typedef struct fonts
{
	int white_rabbit;
	int blazed;
	int avga;
} fonts;

typedef struct sounds
{
	int sound1;
} sounds;

typedef struct resources
{
	images img;
	fonts ft;
	sounds sd;
} resources;

typedef struct aircraft
{
	int style;
	double width;
	double length;
	double x;
	double y;
	double speed;

	size_t total_bullets;
	size_t bullets_used;
	double bullet_speed;
	double bullet_angle;
	double damage;
	double *bullet_x;
	double *bullet_y;
	bool *bullets_hit_flag;

	size_t hit_times;
	size_t dead_limit;
	bool dead_flag;
} aircraft;

////DECLARE ALL GLOBAL VARIABLES

// create an instance of resouces
resources *r, *p;


// PLAYER SETTING
aircraft *player;
size_t player_bullet = 0;
size_t player_dead_limit = 15;
// flag for player to fire
bool player_shoot = false;
bool arrow_up= false;
bool arrow_down = false;
bool arrow_left = false;
bool arrow_right = false;
bool keyQ = false;
bool start = false;
bool end = false;
bool ok = false;

// ENEMY SETTING
aircraft **enm = NULL;
size_t enemy_bullet = 0;
// vector of flags for enemies to shoot
bool *shoot = NULL;
size_t **random = NULL;
size_t shoot_chance;
size_t b_sp;
size_t *type = NULL;

// SOME FLAGS
bool *crash_flag = NULL;
bool **hit_flag = NULL;
bool **is_hit_flag = NULL;

// explosion varibles
double *explode_x = NULL;
double *explode_y = NULL;
size_t *explode_ctr = NULL;

size_t lives = 0;
size_t life_ctr = 0;
size_t total_enemies_killed = 0;
double player_health;
double killed;
double bullet_left;
float score = 0;

// variables for intro windows
enum {EASY, MEDIUM, INSANE} diffculty;
enum {MOUSE, KEYBOARD} control;
enum {QUICK = 3, LONGER = 5} game_time;
char *dif_name[3] = {"EASY", "MEDIUM", "INSANE"};
char *length_name[2] = {"SHORT", "LONG"};

size_t style_choice = 0;
size_t map_choice = 0;
size_t length_choice = 0;

double cursor_y[5] = {40, 40 + 92, 40 + 92*2, 40 + 92*3, 40 + 92*4};
size_t cursor_index = 0;

//// DECLARE ALL FUNCTIONS
resources *load_resources();
aircraft *create_aircraft(int, double, double, double, double, size_t, size_t, size_t, double, double);
aircraft **create_enemy();
void circle_full(int r_f, int g_f, int b_f, double a_f, int r_ol, int g_ol, int b_ol, double a_ol, double x, double y, double radius);
void hit_enemy();
void enemies_move();
void player_move();
void player_attack();
void enemies_attack();
void hit_player();
void crash();
void explode();
void print_stats();
bool game_over();
void bot_setting();
char* rank_score(size_t);
size_t getCursor_y();
void getStyle();
void getMap();
void getDif();
void getLength();
void getControl();

//daclare a array of function pointers for the setting menu
void (*getLine[5])() = {getMap, getControl, getLength, getDif, getStyle};

int main(int args, char *argv[])
{
	srand(time(NULL));
	slWindow(WINDOWS_X, WINDOWS_Y, "SPACE WAR", false);
	//initialize resources
	p = load_resources();
	char prompt[150];
	char *title;
	diffculty = EASY;
	length_choice = 0;
	while(!slShouldClose() && !start && !slGetKey(SL_KEY_ESCAPE))
	{
		slShowCursor(false);
		slSprite((p->img).map[map_choice], WINDOWS_X/2, WINDOWS_Y/2, WINDOWS_X, WINDOWS_Y);
		if (!ok)
		{
			getLine[getCursor_y()]();

			slSprite((p->img).cursor, 80.0, cursor_y[cursor_index], 50.0, 50.0);
			slSprite((p->img).arrows, 100.0, WINDOWS_Y - 250.0, 120.0, 80.0);

			slSprite((p->img).jet[style_choice], WINDOWS_X/2 - 120.0,cursor_y[4], 100.0, 100.0);
			slSprite((p->img).bullet[style_choice], WINDOWS_X/2 - 50, cursor_y[4], 30.0, 60.0);
			// length and dif
			slSprite((p->img).control[control], WINDOWS_X/2 - 50.0, cursor_y[1], 80.0, 50.0);

			title = "SPACE WAR";
			slSetFont((p->ft).avga, 100);
			slSetForeColor(_255to1(255), _255to1(51), _255to1(153), 1.0);
			slText(250, WINDOWS_Y - 150, title);

			sprintf(prompt, "  TO CHOOSE THEN [ENTER]"
							"\n\nSTYLE:"
							"\n\nDIFFICULTY: %s"
							"\n\nLENGTH: %s"
							"\n\nCONTROL:"
							"\n\nMAP",
							dif_name[diffculty], length_name[length_choice]);
			slSetFont((p->ft).white_rabbit, 60);
			slSetForeColor(_255to1(255), _255to1(51), _255to1(51), 1.0);
			slText(120, WINDOWS_Y - 250, prompt);
			slSetForeColor(1.0, 1.0, 1.0, 1.0);

			if (slGetKey(SL_KEY_ENTER)) ok = true;
		}
		else
		{
			title = "HOW TO PLAY";
			slSetFont((p->ft).avga, 100);
			slSetForeColor(_255to1(255), _255to1(51), _255to1(153), 1.0);
			slText(250, WINDOWS_Y - 150, title);
			if (control == MOUSE)
			{
				sprintf(prompt, "TRY NOT TO DIE AND"
								"\n\nKILL ALL ENEMIES"
								"\n\nMOVE BY MOUSE"
								"\n\nSHOOT WITH LEFT-CLICK"
								"\n\nGOOD LUCK!"
								"\n\nLEFT-CLICK TO START GAME ..");
				if (slGetMouseButton(SL_MOUSE_BUTTON_LEFT)) start = true;
			}
			else
			{
				sprintf(prompt, "TRY NOT TO DIE AND "
								"\n\nKILL ALL ENEMIES"
								"\n\nMOVE BY"
								"\n\nSHOOT BY [SPACE]"
								"\n\nGOOD LUCK!"
								"\n\nPRESS [SPACE] TO START GAME ..");
				slSetForeColor(1.0, 1.0, 1.0, 1.0);
				slSprite((p->img).arrows, 400.0, WINDOWS_Y - 400.0, 120.0, 80.0);
				if (slGetKey(' ')) start = true;
			}
			slSetFont((p->ft).white_rabbit, 60);
			slSetForeColor(_255to1(51), _255to1(255), _255to1(153), 1.0);
			slText(50, WINDOWS_Y - 250, prompt);
			slSetForeColor(1.0, 1.0, 1.0, 1.0);
		}

		slRender();
	}
	slClose();

	free(p);

	slWindow(WINDOWS_X, WINDOWS_Y, "GAME PLAY", false);

	r = load_resources();
	
	// PASS THE PLAYER CHOICE AND CREATE THE GAME
	if(length_choice == 0) game_time = QUICK;
	else game_time = LONGER;

	bot_setting();
	enm = create_enemy();
	
	// initialize the player
	player = create_aircraft((r->img).jet[style_choice], 80.0, 80.0, WINDOWS_X/2, 25.0, player_bullet, player_dead_limit, 1.0, 10.0, 0.0);

	// initilize flags
	vector_set_capacity(shoot, vector_size(enm));
	vector_set_capacity(crash_flag, vector_size(enm));
	vector_set_capacity(hit_flag, vector_size(enm));
	vector_set_capacity(is_hit_flag, vector_size(enm));
	vector_set_capacity(random, vector_size(enm));

	for(size_t i = 0; i < vector_size(enm); i++)
	{
		bool *temp1 = NULL;
		bool *temp2 = NULL;
		size_t *temp3 = NULL;
		for(size_t j = 0; j < player_bullet; j++)
			vector_push_back(temp1, false);
		for (size_t k = 0; k < enemy_bullet; k++)
		{
			vector_push_back(temp2, false);
			vector_push_back(temp3, rand()%3 + 1);
		}

		vector_push_back(shoot, false);
		vector_push_back(crash_flag, false);

		vector_push_back(hit_flag, temp1);
		vector_push_back(is_hit_flag, temp2);
		vector_push_back(random, temp3);
	}
	// end flags initialization

	while(!slShouldClose() && !end && !slGetKey(SL_KEY_ESCAPE))
	{
		// hide the cursor
		slShowCursor(false);

		// create background
		slSprite((r->img).map[map_choice], WINDOWS_X/2, WINDOWS_Y/2, WINDOWS_X, WINDOWS_Y);

		player_move();

		player_attack();

		enemies_move();

		hit_enemy();

		enemies_attack();

		hit_player();

		crash();

		explode();

		print_stats();

		game_over();

		slRender();
	}
	slClose();

	return 0;
}

size_t getCursor_y()
{
	if(slGetKey(SL_KEY_UP))
	{
		if(!arrow_up)
		{
			arrow_up = true;
			if (cursor_index < 4) cursor_index++;
			if (cursor_index > 4) cursor_index = 4;
		}
	}
	else arrow_up = false;

	if(slGetKey(SL_KEY_DOWN))
	{
		if(!arrow_down)
		{
			arrow_down = true;
			if (cursor_index > 0) cursor_index--;
			if (cursor_index < 0) cursor_index = 0;
		}
	}
	else arrow_down = false;

	return cursor_index;
}

void getDif()
{
	if(slGetKey(SL_KEY_RIGHT))
	{
		if(!arrow_right)
		{
			arrow_right = true;
			if (diffculty < 2) diffculty++;
			if (diffculty > 2) diffculty = 2;
		}
	}
	else arrow_right = false;

	if(slGetKey(SL_KEY_LEFT))
	{
		if(!arrow_left)
		{
			arrow_left = true;
			if (diffculty > 0) diffculty--;
			if (diffculty < 0) diffculty = 0;
		}
	}
	else arrow_left = false;
}

void getStyle()
{
	if(slGetKey(SL_KEY_RIGHT))
	{
		if(!arrow_right)
		{
			arrow_right = true;
			if (style_choice < 6) style_choice++;
			if (style_choice > 6) style_choice = 6;
		}
	}
	else arrow_right = false;

	if(slGetKey(SL_KEY_LEFT))
	{
		if(!arrow_left)
		{
			arrow_left = true;
			if (style_choice > 0) style_choice--;
			if (style_choice < 0) style_choice = 0;
		}
	}
	else arrow_left = false;
}

void getMap()
{
	if(slGetKey(SL_KEY_RIGHT))
	{
		if(!arrow_right)
		{
			arrow_right = true;
			if (map_choice < 6) map_choice++;
			if (map_choice > 6) map_choice = 6;
		}
	}
	else arrow_right = false;

	if(slGetKey(SL_KEY_LEFT))
	{
		if(!arrow_left)
		{
			arrow_left = true;
			if (map_choice >0) map_choice--;
			if (map_choice < 0) map_choice = 0;
		}
	}
	else arrow_left = false;
}

void getLength()
{
	if(slGetKey(SL_KEY_RIGHT))
	{
		if(!arrow_right)
		{
			arrow_right = true;
			length_choice = 1;
		}
	}
	else arrow_right = false;

	if(slGetKey(SL_KEY_LEFT))
	{
		if(!arrow_left)
		{
			arrow_left = true;
			length_choice = 0;
		}
	}
	else arrow_left = false;
}

void getControl()
{
	if(slGetKey(SL_KEY_RIGHT))
	{
		if(!arrow_right)
		{
			arrow_right = true;
			control = 1;
		}
	}
	else arrow_right = false;

	if(slGetKey(SL_KEY_LEFT))
	{
		if(!arrow_left)
		{
			arrow_left = true;
			control = 0;
		}
	}
	else arrow_left = false;
}

void bot_setting()
{
	// create each enemy, here we use a loop to specify all enemy the same
	for(size_t i = 0; i < game_time;  i++)
	{
		if (diffculty == EASY)
		{
			vector_push_back(type, 5);
			shoot_chance = 100;
			b_sp = 5.0;
			lives++;
			player_bullet += (5*(i+1)+i+5) + 2; // enough bullet to kill all and some reserve
			enemy_bullet += i*20*6;
		}
		else if (diffculty == MEDIUM)
		{
			vector_push_back(type, 7);
			lives++;
			b_sp = 7.0;
			shoot_chance = 50;
			player_bullet += (7*(i+1)+i+5) + 5; // enough bullet to kill all and some reserve
			enemy_bullet += i*20*8;
		}
		else if (diffculty == INSANE)
		{
			vector_push_back(type, 10);
			lives++;
			b_sp = 10.0;
			shoot_chance = 10;
			player_bullet += (10*(i+1)+i+5) + 10; // enough bullet to kill all and some reserve
			enemy_bullet += i*20*11;
		}
	}

}

bool game_over()
{
	if (life_ctr > lives) life_ctr = lives;
	if (life_ctr == lives) 
		return true;
	if (total_enemies_killed == vector_size(enm))
	{
		killed = 1.0;
		return true;
	}
	else return false;
}

void print_stats()
{
	if (!game_over())
	{
		player_health = 1.0 - ((float) player->hit_times) / player->dead_limit;
		if (player_health < 0) player_health = 0;
		if(player_health == 0.0)
		{
			slRender();
			player->hit_times = 0;
			life_ctr++;
		}
		killed = (float)total_enemies_killed/vector_size(enm);

		bullet_left = (float) player->total_bullets - player->bullets_used;
	}
	else
	{
		for (size_t u = 0; u < vector_size(enm); u++)
			if(!enm[u]->dead_flag)
				enm[u]->dead_flag = true;

		if (player->bullets_used < player->total_bullets) 
			player->bullets_used = player->total_bullets;
		char *rank;
		if (score < 1000*((killed + (1-1.0*life_ctr/lives))/2))
		{
			score++;
			rank = "CALCULATING ...";
		}
		else
		{
			rank = rank_score(score);
			if(slGetKey(SL_KEY_ENTER)) end = true;
		}
		char *end_title = "GAME OVER";
		slSetFont((r->ft).blazed, 75);
		slSetForeColor(_255to1(200), _255to1(200), 0.0, 1.0);
		slText(10, WINDOWS_Y - 180.0, end_title);

		char game_over_prompt[100];
		sprintf(game_over_prompt,"DIFFICULTY: %s"
								"\n\nMODE: %s"
								"\n\nSCORE: %.f"
								"\n\nRANKING: %s"
								"\n\nPRESS [ENTER] TO QUIT",
								dif_name[diffculty], length_name[length_choice], score, rank);
		slSetFont((r->ft).white_rabbit, 65);
		slSetForeColor(_255to1(0), _255to1(200), _255to1(100), 1.0);
		slText(50.0, WINDOWS_Y - 300.0, game_over_prompt);

		slSetForeColor(1.0, 1.0, 1.0, 1.0);
	}
	char HEALTH[50];

	sprintf(HEALTH, "HEALTH: %.f%%", player_health*100.0);
	slSetFont((r->ft).white_rabbit, 20);
	slSetForeColor(1.0, 1.0, 1.0, 1.0);
	slText(WINDOWS_X - 150, WINDOWS_Y - 50, HEALTH);

	slSetForeColor(1.0, 1.0, 0.0, 0.5);
	slRectangleOutline(WINDOWS_X - 150, WINDOWS_Y - 20, 275, 20);

	slPush();
	slTranslate(-275*(1-player_health)/2, 0.0);
	slSetForeColor(1.0, 0.0, 0.0, 1.0);
	slRectangleFill(WINDOWS_X - 150 + 275/2, WINDOWS_Y - 20, 275*(1- player_health), 20);
	slPop();

	slPush();
	slTranslate(275*player_health/2, 0.0);
	slSetForeColor(0.0, 1.0, 0.0, 1.0);
	slRectangleFill(WINDOWS_X - 150 - 275/2, WINDOWS_Y - 20, 275*player_health, 20);
	slPop();
	slSetForeColor(1.0, 1.0, 1.0, 1.0);

	char KILL[50];
	sprintf(KILL, "ENEMIES KILLED: %.f%%", killed*100.0);
	slSetFont((r->ft).white_rabbit, 20);
	slSetForeColor(1.0, 1.0, 1.0, 1.0);
	slText(WINDOWS_X/2 - 150.0, WINDOWS_Y - 50.0, KILL);

	slSetForeColor(1.0, 1.0, 0.0, 1.0);
	slRectangleOutline(WINDOWS_X/2 - 50.0, WINDOWS_Y - 20.0, 500.0, 20);

	slPush();
	slTranslate(250*killed, 0.0);
	slSetForeColor(0.0, 0.0, _255to1(153), 1.0);
	slRectangleFill(WINDOWS_X/2 - 300.0, WINDOWS_Y - 20.0, 500.0*killed, 20);
	slPop();

	slPush();
	slTranslate(250*(killed-1), 0.0);
	slSetForeColor(_255to1(102), _255to1(102), _255to1(255), 1.0);
	slRectangleFill(WINDOWS_X/2 + 200.0, WINDOWS_Y - 20, 500*(1-killed), 20);
	slPop();
	slSetForeColor(1.0, 1.0, 1.0, 1.0);

	char BULLET[50];

	sprintf(BULLET, "Bullet: %.f", bullet_left);
	slSetFont((r->ft).white_rabbit, 20);
	slSetForeColor(1.0, 1.0, 1.0, 1.0);
	slText(15, WINDOWS_Y - 50, BULLET);

	slSetForeColor(1.0, 1.0, 0.0, 0.5);
	slRectangleOutline(100.0, WINDOWS_Y - 20, 180.0, 20);

	slSetForeColor(1.0, 1.0, 1.0, 1.0);

	slPush();
	slTranslate(90*(bullet_left/player->total_bullets), 0.0);
	slSetForeColor(_255to1(102), _255to1(102), 0.0, 1.0);
	slRectangleFill(10.0, WINDOWS_Y - 20, 180*(bullet_left/player->total_bullets), 20);
	slPop();

	slPush();
	slTranslate(-90*(1-bullet_left/player->total_bullets), 0.0);
	slSetForeColor(_255to1(255), _255to1(255), _255to1(153), 1.0);
	slRectangleFill(190.0, WINDOWS_Y - 20, 180*(1-bullet_left/player->total_bullets), 20);
	slPop();

	if (lives > life_ctr)
		for (size_t i = 0; i < lives - life_ctr; i++)
			slSprite((r->img).heart,  WINDOWS_X - 30 - 50.0*i, WINDOWS_Y - 80, 40.0, 40.0);

	slSetForeColor(1.0, 1.0, 1.0, 1.0);
}

char *rank_score(size_t score)
{
	char *rank;
	if(life_ctr < lives)
	{
		if (score >= 900) rank = "LEGENDARY";
		else if (score < 900 && score >= 700) rank = "MASTER";
		else if (score < 700 && score >= 500) rank = "WELL PLAYED";
		else rank = "LUCKY";
	}
	else
		rank = "LOSER";
	return rank;
}

void crash()
{
	for (size_t u = 0; u < vector_size(enm); u++)
	{
		double distance_x = abs(enm[u]->x - player->x);
		double distance_y = abs(enm[u]->y - player->y);
		// if(distance_x < (player->width+enm[u]->width)/2 && distance_y < (player->length +enm[u]->length)/2)
		if (distance_x < 60 && distance_y < 50)
		{
			if(!crash_flag[u])
			{
				player->hit_times += enm[u]->damage;
				vector_push_back(explode_x, player->x);
				vector_push_back(explode_y, player->y);
				vector_push_back(explode_ctr, 0);

				enm[u]->hit_times += player->damage;
				vector_push_back(explode_x, enm[u]->x);
				vector_push_back(explode_y, enm[u]->y);
				vector_push_back(explode_ctr, 0);
				if (enm[u]->hit_times == enm[u]->dead_limit)
				{
					enm[u]->dead_flag = true;
					total_enemies_killed++;
				}
				// crash_flag[u] = true;
			}

		}
		else crash_flag[u] = false;
	}
}

void explode()
{
	for (size_t i = 0; i < vector_size(explode_x); i++)
	{
		if(explode_ctr[i] <= 3)
		{
			slSprite((r->img).explode, explode_x[i], explode_y[i], 100.0, 100.0);
			explode_ctr[i]++;
		}
		else
		{
			vector_erase(explode_x, i);
			vector_erase(explode_y, i);
			vector_erase(explode_ctr, i);
		}
	}
}

void enemies_attack()
{
	for(size_t u = 0; u < vector_size(enm); u++)
	{
		if (rand() % shoot_chance == 7) // MODIFY DIFFICULTY HERE AND AT NUMBER OF ENEMIES
		{
			if(enm[u]->y < WINDOWS_Y)
			{
				if (!shoot[u])
				{
					shoot[u] = true;
					if (enm[u]->bullets_used < enm[u]->total_bullets)
					{
						enm[u]->bullet_x[enm[u]->bullets_used] = enm[u]->x;
						enm[u]->bullet_y[enm[u]->bullets_used] = enm[u]->y - (enm[u]->length)/2;
						if (enm[u]->bullets_used == enm[u]->total_bullets - 1) 
							enm[u]->bullets_used = 0;
						else enm[u]->bullets_used++;
					}
				}
			}
		}
		else shoot[u] = false;

		for (size_t i = 0; i < enm[u]->total_bullets; i++)
		{
			if (!enm[u]->bullets_hit_flag[i])
			{
				if (enm[u]->bullet_x[i] > 0.0 && enm[u]->bullet_x[i] < WINDOWS_X)
				{
					if (enm[u]->bullet_y[i] > 0.0 && enm[u]->bullet_y[i] < WINDOWS_Y)
					{
						circle_full(255, 255, 51, 1.0, 255, 0, 0, 1.0, enm[u]->bullet_x[i], enm[u]->bullet_y[i], 10); // design the bullet here
						
						enm[u]->bullet_y[i] -= enm[u]->bullet_speed; // control the speed of the bullets

						if (random[u][i] == 1)
							enm[u]->bullet_x[i] -= 0.0;
						if(random[u][i] == 2)
							enm[u]->bullet_x[i] -= enm[u]->bullet_angle;
						if(random[u][i] == 3)
							enm[u]->bullet_x[i] += enm[u]->bullet_angle;
						//DIFFERET DIRECTIONS OF BULLETS FOR MORE DIFFICULTY
					}
				}
			}
			else
			{
				enm[u]->bullet_y[i] = 0.0;
				enm[u]->bullet_x[i] = 0.0;
			}
		}
	}
	slSetForeColor(1.0, 1.0, 1.0, 1.0);
}

void hit_player()
{
	for (size_t u = 0; u < vector_size(enm); u++)
	{
		for (size_t i = 0; i < enm[u]->total_bullets; i++)
		{
			double distance_x = abs(enm[u]->bullet_x[i] - player->x);
			double distance_y = abs(enm[u]->bullet_y[i] - player->y);
			if(distance_x < 40 && distance_y < 40)
			{
				if(!is_hit_flag[u][i])
				{
					enm[u]->bullets_hit_flag[i] = true;
					player->hit_times += enm[u]->damage;
					vector_push_back(explode_x, player->x);
					vector_push_back(explode_y, player->y);
					vector_push_back(explode_ctr, 0);

					is_hit_flag[u][i] = true;
				}
			}
			else is_hit_flag[u][i] = false;
		}
	}
}

void player_attack()
{
	// link left-mouse to shooting
	if (control == KEYBOARD)
	{
		if (slGetKey(' '))
		{
			if (!player_shoot)
			{
				player_shoot = true;
				if (player->bullets_used < player->total_bullets)
				{
					player->bullet_x[player->bullets_used] = player->x;
					player->bullet_y[player->bullets_used] = player->y + (player->length)/2;
					player->bullets_used++;
				}
			}
		}
		else player_shoot = false;
	}
	else 
	{
		if (slGetMouseButton(SL_MOUSE_BUTTON_LEFT))
		{
			if (!player_shoot)
			{
				player_shoot = true;
				if (player->bullets_used < player->total_bullets)
				{
					player->bullet_x[player->bullets_used] = player->x;
					player->bullet_y[player->bullets_used] = player->y + (player->length)/2;
					player->bullets_used++;
				}
			}
		}
		else player_shoot = false;
	}

	//show bullet with the left-mouse condition
	for (size_t i = 0; i < player->total_bullets; i++)
	{
		if (!player->bullets_hit_flag[i])
		{
			if (player->bullet_x[i] > 0.0 && player->bullet_x[i] < WINDOWS_X)
			{
				if (player->bullet_y[i] > 0.0 && player->bullet_y[i] < WINDOWS_Y)
				{
					slSprite((r->img).bullet[style_choice], player->bullet_x[i], player->bullet_y[i], 10, 40);
					player->bullet_y[i] += player->bullet_speed; // control the speed of the bullets
					player->bullet_x[i] += player->bullet_angle;  // control the direction of the bullets
				}
				
			}
		}
		else
		{
			player->bullet_y[i] = 0.0;
			player->bullet_x[i] = 0.0;
		}
	}
	slSetForeColor(1.0, 1.0, 1.0, 1.0);
}

void enemies_move()
{
	// make enemies alive
	for (size_t u = 0; u < vector_size(enm); u++)
	{
		if(!enm[u]->dead_flag)
		{
			slPush();
			slTranslate(enm[u]->x, enm[u]->y);
			slRotate(180.0);
			slSprite(enm[u]->style, 0.0, 0.0, enm[u]->width, enm[u]->length);
			slPop();
			
			enm[u]->x -= 0.0; // control the direction of the enemies
			enm[u]->y -= enm[u]->speed; // control the speed of the enemies
			if(enm[u]->y <= 0.0) 
			for(size_t k = 0; k < vector_size(enm); k++)
				enm[k]->y += WINDOWS_Y + 100.0; // move the enemies back to frame
		}
		else
			enm[u]->y = -10000.0;
	}
}

void hit_enemy()
{
	for (size_t u = 0; u < vector_size(enm); u++)
	{
		for (size_t i = 0; i < player->total_bullets; i++)
		{
			double distance_x = abs(player->bullet_x[i] - enm[u]->x);
			double distance_y = abs(player->bullet_y[i] - enm[u]->y);
			// if(distance_x < enm[u]->width/2 && distance_y < enm[u]->length/2)
			if (distance_x < 50 && distance_y < 40)
			{
				if(!hit_flag[u][i])
				{
					player->bullets_hit_flag[i] = true;
					enm[u]->hit_times += player->damage;
					vector_push_back(explode_x, enm[u]->x);
					vector_push_back(explode_y, enm[u]->y);
					vector_push_back(explode_ctr, 0);
					if (enm[u]->hit_times >= enm[u]->dead_limit)
					{
						enm[u]->dead_flag = true;
						total_enemies_killed++;
					}
					hit_flag[u][i] = true;
				}
			}
			else hit_flag[u][i] = false;
		}
	}
}

void player_move()
{
	if(control == KEYBOARD)
	{
		if (slGetKey(SL_KEY_UP))
			player->y += 5.0;
		else arrow_up = false;
		if (slGetKey(SL_KEY_DOWN))
			player->y -= 5.0;
		else arrow_down = false;
		if (slGetKey(SL_KEY_LEFT))
			player->x -= 5.0;
		if (slGetKey(SL_KEY_RIGHT))
			player->x += 5.0;
	}
	else if (control == MOUSE)
	{
		player->x = slGetMouseX();
		player->y = slGetMouseY();
	}
	// create player and give them mouse movement;
	if (player->x < player->width/2) player->x = player->width/2;
	if (player->x > WINDOWS_X - player->width/2) player->x = WINDOWS_X - player->width/2;
	if (player->y < player->length/2) player->y = player->length/2;
	if (player->y > WINDOWS_Y - player->length/2) player->y = WINDOWS_Y - player->length/2;

	slSprite(player->style, player->x, player->y, player->width, player->length);
}

void circle_full(int r_f, int g_f, int b_f, double a_f, int r_ol, int g_ol, int b_ol, double a_ol, double x, double y, double radius)
{
	slSetForeColor(_255to1(r_f), _255to1(g_f), _255to1(b_f), a_f);
	slCircleFill( x, y, radius, 25);	
	
	slSetForeColor(_255to1(r_ol), _255to1(g_ol), _255to1(b_ol), a_ol);
	slCircleOutline( x, y, radius, 25);
}

aircraft **create_enemy()
{
	// create a vector to store all enemies pointer, set size as passed
	aircraft *(*enm_vector) = NULL;
	vector_set_capacity(enm_vector, 60);
	// change here to specify each enemy differently
	for (size_t k = 0; k < vector_size(type); k++)
	{
		for (size_t i = 0; i < type[k]; i++)
		{
			size_t index = rand()%7;
			aircraft *an_enm = create_aircraft((r->img).bot[index], 70.0, 70.0, (WINDOWS_X/(type[k] + 1))*(i+1), WINDOWS_Y*1.1*(2*k+1), 20*k, k+1, k+1, b_sp, (float)index*k);
			vector_push_back(enm_vector, an_enm);
		}
		// BOSS HERE
		size_t index = rand()%7;
		aircraft *an_enm = create_aircraft((r->img).bot[index], 450.0, 100.0, WINDOWS_X/2, WINDOWS_Y*1.1*(2*k+2), 20*k, k+5, k+3, b_sp, (float)index*k);
		vector_push_back(enm_vector, an_enm);
	}
	return enm_vector;
}

aircraft *create_aircraft(int img, double w, double l, double pos_x, double pos_y, size_t tb, size_t dl, size_t dmg, double b_sp, double b_agl)
{
	aircraft* new;
	new = malloc(sizeof(aircraft));
	if (new == NULL) 
	{
		perror("");
		return NULL;
	}
	new->style = img;
	new->width = w;
	new->length = l;
	new->x = pos_x;
	new->y = pos_y;
	new->total_bullets = tb;
	new->bullets_used = 0;
	new->dead_limit = dl;
	new->dead_flag = false;
	new->hit_times = 0;
	new->speed = 3.0;
	new->bullet_speed = b_sp;
	new->bullet_angle = b_agl;
	new->damage = dmg;

	new->bullet_x = NULL;
	new->bullet_y = NULL;
	new->bullets_hit_flag = NULL;
	vector_set_capacity(new->bullet_x, tb);
	vector_set_capacity(new->bullet_y, tb);
	vector_set_capacity(new->bullets_hit_flag, tb);
	for(size_t i = 0; i < tb; i++)
	{
		vector_push_back(new->bullet_x, 0); // initial position of bullets
		vector_push_back(new->bullet_y, 0);
		vector_push_back(new->bullets_hit_flag, false);
	}
	return new;
}

resources *load_resources()
{
	resources *new;
	new = malloc(sizeof(resources));
	if (new == NULL) 
	{
		perror("");
		return NULL;
	}
	int (*LOAD_DATA)(const char *);

	LOAD_DATA = slLoadTexture;
	(new->img).map[0]= (*LOAD_DATA)("Images/map0.jpg");
	(new->img).map[1]= (*LOAD_DATA)("Images/map1.jpg");
	(new->img).explode = (*LOAD_DATA)("Images/explosion.png");
	(new->img).heart = (*LOAD_DATA)("Images/heart.png");
	(new->img).control[0] = (*LOAD_DATA)("Images/mouse.png");
	(new->img).control[1] = (*LOAD_DATA)("Images/keyboard.png");
	(new->img).arrows = (*LOAD_DATA)("Images/arrows.png");
	(new->img).cursor = (*LOAD_DATA)("Images/select.png");

	for(size_t i = 0; i < 7; i++)
	{
		char jet[20], bullet[20], bot[20], map[20];
		sprintf(jet, "Images/jet%u.png", i);
		(new->img).jet[i] = (*LOAD_DATA)(jet);
		sprintf(bullet, "Images/bullet%u.png", i);
		(new->img).bullet[i] = (*LOAD_DATA)(bullet);
		sprintf(bot, "Images/bot%u.png", i);
		(new->img).bot[i] = (*LOAD_DATA)(bot);
		sprintf(map, "Images/map%u.jpg", i);
		(new->img).map[i] = (*LOAD_DATA)(map);
	}

	LOAD_DATA = slLoadFont;
	(new->ft).white_rabbit = (*LOAD_DATA)("Fonts/white_rabbit.ttf");
	(new->ft).blazed = (*LOAD_DATA)("Fonts/blazed.ttf");
	(new->ft).avga = (*LOAD_DATA)("Fonts/avga.ttf");
	return new;
}
