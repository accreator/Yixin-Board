#define _CRT_SECURE_NO_WARNINGS /* for vs */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "yixin.h"
#include "resource.h" /* not needed if you are not provided with resource.h, Yixin.rc and icon.ico */

typedef long long I64;
#define MAX_SIZE 24
#define CAUTION_NUM 4  //0..CAUTION_NUM
#define MIN_SPLIT_DEPTH 5
#define MAX_SPLIT_DEPTH 20
#define MAX_TOOLBAR_ITEM 32
#define MAX_TOOLBAR_COMMAND_LEN 2048
#define MAX_HOTKEY_ITEM 32
#define MAX_HOTKEY_COMMAND_LEN 2048

typedef struct
{
	I64 zobkey;
	char status;
} BOOK;
BOOK *openbook = NULL;
int openbooksize = 0;
int openbooknum = 0;
int zobristflag = 1;
I64 zobrist[MAX_SIZE][MAX_SIZE][3];
int boardsizeh = 15, boardsizew = 15;
int rboardsizeh = 15, rboardsizew = 15;
int inforule = 0;
int specialrule = 0;
int infopondering = 0;
int infocheckmate = 0;
int timeoutturn = 10000;
int timeused = 0;
int timestart = 0;
int timeoutmatch = 2000000;
int maxdepth = 100;
int maxnode = 1000000000;
int maxthreadnum = 1; //1..maxthreadnum
int maxhashsize = 21;
int computerside = 0; /* 0 none 1 black 2 while 3 black&white */
int cautionfactor = 1;
int threadnum = 1;
int hashsize = 19;
int threadsplitdepth = 7;
int board[MAX_SIZE][MAX_SIZE];
int boardnumber[MAX_SIZE][MAX_SIZE];
int movepath[MAX_SIZE*MAX_SIZE];
int forbid[MAX_SIZE][MAX_SIZE];
int boardblock[MAX_SIZE][MAX_SIZE];
int boardbest[MAX_SIZE][MAX_SIZE];
int boardlose[MAX_SIZE][MAX_SIZE];
int boardpos[MAX_SIZE][MAX_SIZE];
int blockautoreset = 0;
int blockpathautoreset = 0;
int hashautoclear = 0;
int piecenum = 0;
char isthinking = 0, isgameover = 0, isneedrestart = 0, isneedomit = 0;
char bestline[MAX_SIZE*MAX_SIZE*5+1] = "";
int bestval;
int useopenbook = 1;
int levelchoice = 4;
int commandmodel = 0;
int shownumber = 1;
int showlog = 1;
int showanalysis = 1;
int showtoolbarboth = 1;
int showsmallfont = 0;
int showwarning = 1;
int toolbarpos = 1;
int language = 0; /* 0: English 1: Other languages */
int rlanguage = 0;
char **clanguage = NULL; /* Custom language */
int movx[8] = {  0,  0,  1, -1,  1,  1, -1, -1}; /* note that the order is related to winning checking function(s)*/
int movy[8] = {  1, -1,  0,  0,  1, -1,  1, -1};
/* engine */
GIOChannel *iochannelin, *iochannelout, *iochannelerr;
/* windowmain */
GtkWidget *windowmain;
GtkWidget *tableboard;
GtkWidget *imageboard[MAX_SIZE][MAX_SIZE];
GtkWidget *labelboard[2][MAX_SIZE];
GtkWidget *vboxwindowmain;
GdkPixbuf *pixbufboard[9][12];
GdkPixbuf *pixbufboardnumber[9][12][MAX_SIZE*MAX_SIZE+1][2];
int imgtypeboard[MAX_SIZE][MAX_SIZE];
char piecepicname[80] = "piece.bmp";
/* log */
GtkWidget *textlog;
GtkTextBuffer *buffertextlog, *buffertextcommand;
GtkWidget *scrolledtextlog, *scrolledtextcommand;
GtkWidget *toolbar;

int toolbarnum = 6;

int toolbarlng[MAX_TOOLBAR_ITEM] =
{
	48,
	46,
	47,
	49,
	45,
	44
};

char *toolbaricon[MAX_TOOLBAR_ITEM] =
{
	GTK_STOCK_GOTO_FIRST,
	GTK_STOCK_GO_BACK,
	GTK_STOCK_GO_FORWARD,
	GTK_STOCK_GOTO_LAST,
	GTK_STOCK_STOP,
	GTK_STOCK_EXECUTE
};

char toolbarcommand[MAX_TOOLBAR_ITEM][MAX_TOOLBAR_COMMAND_LEN] =
{
	"undo all\n",
	"undo one\n",
	"redo one\n",
	"redo all\n",
	"thinking stop\n",
	"thinking start\n"
};

int hotkeynum = 6;

int hotkeykey[MAX_HOTKEY_ITEM] =
{
	13,
	14,
	15,
	16,
	27,
	11
};

char hotkeycommand[MAX_HOTKEY_ITEM][MAX_HOTKEY_COMMAND_LEN] =
{
	"undo all\n",
	"redo all\n",
	"undo one\n",
	"redo one\n",
	"thinking stop\n",
	"thinking start\n"
};

char *hotkeynamelis[] = {
	"",
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
	"Ctrl + Up", "Ctrl + Down", "Ctrl + Left", "Ctrl + Right",
	"Ctrl + 1", "Ctrl + 2", "Ctrl + 3", "Ctrl + 4", "Ctrl + 5", "Ctrl + 6", "Ctrl + 7", "Ctrl + 8", "Ctrl + 9", "Ctrl + 0",
	"Escape", NULL
};

int hotkeykeylis[][2] =
{
	{ 0, 0 }, //0
	{ 0, GDK_F1 }, //1
	{ 0, GDK_F2 }, //2
	{ 0, GDK_F3 }, //3
	{ 0, GDK_F4 }, //4
	{ 0, GDK_F5 }, //5
	{ 0, GDK_F6 }, //6
	{ 0, GDK_F7 }, //7
	{ 0, GDK_F8 }, //8
	{ 0, GDK_F9 }, //9
	{ 0, GDK_F10 }, //10
	{ 0, GDK_F11 }, //11
	{ 0, GDK_F12 }, //12

	{ GDK_CONTROL_MASK, GDK_Up }, //13
	{ GDK_CONTROL_MASK, GDK_Down }, //14
	{ GDK_CONTROL_MASK, GDK_Left }, //15
	{ GDK_CONTROL_MASK, GDK_Right }, //16

	{ GDK_CONTROL_MASK, GDK_1 }, //17
	{ GDK_CONTROL_MASK, GDK_2 }, //18
	{ GDK_CONTROL_MASK, GDK_3 }, //19
	{ GDK_CONTROL_MASK, GDK_4 }, //20
	{ GDK_CONTROL_MASK, GDK_5 }, //21
	{ GDK_CONTROL_MASK, GDK_6 }, //22
	{ GDK_CONTROL_MASK, GDK_7 }, //23
	{ GDK_CONTROL_MASK, GDK_8 }, //24
	{ GDK_CONTROL_MASK, GDK_9 }, //25
	{ GDK_CONTROL_MASK, GDK_0 }, //26

	{ 0, GDK_Escape } //27
};

char * _T(char *s)
{
	return g_locale_to_utf8(s, -1, 0, 0, 0);
}

void print_log(char *text)
{
	GtkTextIter start, end;
	GtkTextMark *endmark;
	static int init = 0;
	static int flag = 0, fspace = 0;
	int len;
	int i;

	if(buffertextlog == NULL)
	{
		printf("%s", text);
		return;
	}

	if (commandmodel == 0)
	{
		if (strncmp(text, "OK", 2) == 0) text += 2;
		if (strncmp(text, "MESSAGE", 7) == 0) text += 7 + 1;
		if (strncmp(text, "DETAIL", 6) == 0) text += 6 + 1;
		if (strncmp(text, "DEBUG", 5) == 0) text += 5 + 1;
		if (strncmp(text, "ERROR", 5) == 0) text += 5;
		if (strncmp(text, "UNKNOWN", 7) == 0) text += 7;
		//if(strncmp(text, "FORBID", 6) == 0) text += 6;
		if (strncmp(text, "YIXIN", 5) == 0 || strncmp(text, "DEEP YIXIN", 10) == 0)
		{
			if (flag == 0) flag = 1; else return;
		}

		for (i = 0; text[i]; i++)
		{
			if (!isspace(text[i])) break;
		}
		if (text[i] == 0)
		{
			fspace++;
		}
		else
		{
			fspace = 0;
		}
		if (fspace >= 2) return;
	}

	len = gtk_text_buffer_get_line_count(GTK_TEXT_BUFFER(buffertextlog));
	if(len > 400)
	{
		gtk_text_buffer_get_iter_at_line(GTK_TEXT_BUFFER(buffertextlog), &start, 0);
		gtk_text_buffer_get_iter_at_line(GTK_TEXT_BUFFER(buffertextlog), &end, len-400);
		gtk_text_buffer_delete(GTK_TEXT_BUFFER(buffertextlog), &start, &end);
	}
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffertextlog), &start, &end);
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffertextlog), &end, text, strlen(text));

	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(buffertextlog), &end);
	gtk_text_iter_set_line_offset(&end, 0);
	if(init == 0)
	{
		init = 1;
		gtk_text_buffer_create_mark(GTK_TEXT_BUFFER(buffertextlog), "scroll", &end, TRUE);
	}
	endmark = gtk_text_buffer_get_mark(GTK_TEXT_BUFFER(buffertextlog), "scroll");
	gtk_text_buffer_move_mark(GTK_TEXT_BUFFER(buffertextlog), endmark, &end);
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(textlog), endmark);
}

int printf_log(char *fmt, ...)
{
	int cnt;
	char buffer[8192];
	char text[8192], *p = text;
	int i;
	va_list va;
	va_start(va,fmt);
	cnt = vsprintf(buffer, fmt, va);

	if(language)
	{
		for(i=0; buffer[i]; )
		{
			if(strncmp(buffer+i, "BESTLINE", 8) == 0)
			{
				strcpy(p, clanguage[0]);
				p += strlen(clanguage[0]);
				i += 8;
				continue;
			}
			/* removed from new engine
			if(strncmp(buffer+i, "COMPLEXITY", 10) == 0)
			{
				strcpy(p, "局势复杂度");
				p += 10;
				i += 10;
				continue;
			}
			*/
			if(strncmp(buffer+i, "EVALUATION", 10) == 0)
			{
				strcpy(p, clanguage[1]);
				p += strlen(clanguage[1]);
				i += 10;
				continue;
			}
			if(strncmp(buffer+i, "SPEED", 5) == 0)
			{
				strcpy(p, clanguage[2]);
				p += strlen(clanguage[2]);
				i += 5;
				continue;
			}
			if(strncmp(buffer+i, "TIME", 4) == 0)
			{
				strcpy(p, clanguage[3]);
				p += strlen(clanguage[3]);
				i += 4;
				continue;
			}
			if(strncmp(buffer+i, "DEPTH", 5) == 0)
			{
				strcpy(p, clanguage[4]);
				p += strlen(clanguage[4]);
				i += 5;
				continue;
			}
			if(strncmp(buffer+i, "BLOCK", 5) == 0)
			{
				strcpy(p, clanguage[5]);
				p += strlen(clanguage[5]);
				i += 5;
				continue;
			}
			if(strncmp(buffer+i, "NODE", 4) == 0)
			{
				strcpy(p, clanguage[6]);
				p += strlen(clanguage[6]);
				i += 4;
				continue;
			}
			if(strncmp(buffer+i, "VAL", 3) == 0)
			{
				strcpy(p, clanguage[7]);
				p += strlen(clanguage[7]);
				i += 3;
				continue;
			}
			if(strncmp(buffer+i, "MS", 2) == 0)
			{
				strcpy(p, clanguage[8]);
				p += strlen(clanguage[8]);
				i += 2;
				continue;
			}
			if(strncmp(buffer+i, "RULE", 4) == 0)
			{
				strcpy(p, clanguage[9]);
				p += strlen(clanguage[9]);
				i += 4;
				continue;
			}
			*p = buffer[i];
			p ++;
			i ++;
		}
		*p = '\0';
	}
	else
	{
		strcpy(text, buffer);
	}

	print_log(_T(text));
	va_end(va);
	return cnt;
}

void print_command(char *text)
{
	GtkTextIter start, end;

	if(buffertextcommand == NULL)
	{
		printf("%s", text);
		return;
	}

	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffertextcommand), &start, &end);
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffertextcommand), &end, text, strlen(text));
}

int printf_command(char *fmt, ...)
{
	int cnt;
	char buffer[1024];
	va_list va;
	va_start(va,fmt);
	cnt = vsprintf(buffer, fmt, va);
	print_command(_T(buffer));
	va_end(va);
	return cnt;
}

void show_welcome()
{
	printf_log("Yixin Board "VERSION"\n");
	if(language)
	{
		printf_command(clanguage[10]);
	}
	else
	{
		printf_command("To get help, type help and press Enter here");
	}
}

void show_thanklist()
{
	printf_log(language==0?"I would like to thank my contributors, whose support makes Yixin what it is.":
			clanguage[11]);
	printf_log("\n");
	printf_log("  彼方\n");
	printf_log("  XR\n");
	printf_log("  舒自均\n");
	printf_log("  Tianyi Hao\n");
	printf_log("  吴豪\n");
	printf_log("  雨中飞燕\n");
	printf_log("  Tuyen Do\n");
	printf_log("  肥国乃乃\n");
	printf_log("  Saturn|Titan\n");
	printf_log("  元\n");
	printf_log("  Alexander Bogatirev\n");
	printf_log("  TZ\n");
	printf_log("  濤声依旧\n");
	printf_log("  张锡森\n");
	printf_log("  ax_pokl\n");
	printf_log("  Ola Strom");
	printf_log("\n");
}

GdkPixbuf *draw_overlay(GdkPixbuf *pb, int w, int h, gchar *text, char *color, int type)
{
	GdkPixmap *pm;
	GdkGC *gc;
	GtkWidget *scratch;
	PangoLayout *layout;
	gchar *markup;
	GdkPixbuf *ret;
	gchar format[100];
	
	pm = gdk_pixmap_new(windowmain->window, w, h, -1);
	gdk_drawable_set_colormap(pm, gdk_colormap_get_system());
	gc = gdk_gc_new(pm);
	gdk_draw_pixbuf(pm, gc, pb, 0, 0, 0, 0, w, h, GDK_RGB_DITHER_NONE, 0, 0);
	scratch = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_realize(scratch);
	layout = gtk_widget_create_pango_layout(scratch, NULL);
	gtk_widget_destroy(scratch);
	if(type == 0)
		sprintf(format, "<big><b><span foreground='%s'>%%s</span></b></big>", color);
	else
		sprintf(format, "<b><span foreground='%s'>%%s</span></b>", color);
	markup = g_strdup_printf(format, text);
	pango_layout_set_markup(layout, markup, -1);
	g_free(markup);
	if(type == 0)
		gdk_draw_layout(pm, gc, w/2-strlen(text)*4, h/2-10, layout);
	else
		gdk_draw_layout(pm, gc, w/2-strlen(text)*4, h/2-8, layout);
	g_object_unref(layout);
	ret = gdk_pixbuf_get_from_drawable(NULL, pm, NULL, 0, 0, 0, 0, w, h);
	return ret;
}

void send_command(char *command)
{
	gsize size;
	g_io_channel_write_chars(iochannelin, command, -1, &size, NULL);
	g_io_channel_flush(iochannelin, NULL);
	//printf_log(command); //for debug
}

int refreshboardflag = 0; //it is set to 1 when making the 5th move under rif rule, otherwise, 0
void refresh_board()
{
	int i, j;
	for(i=0; i<boardsizeh; i++)
	{
		for(j=0; j<boardsizew; j++)
		{
			if(board[i][j] == 0)
			{
				int f = 0;
				if(inforule == 2 && (computerside&1)==0 && piecenum%2==0 && forbid[i][j] && isgameover==0 && isthinking==0) f = 2;
				if(f == 0)
				{
					if(boardblock[i][j]) f = 10;
					else if(showanalysis)
					{
						if(boardlose[i][j]) f = 7;
						else if(boardbest[i][j]) f = 8;
						else if(boardpos[i][j] == 1) f = 9;
						else if(boardpos[i][j] == 2) f = 11;
					}
				}
				if(imgtypeboard[i][j] <= 8)
					gtk_image_set_from_pixbuf(GTK_IMAGE(imageboard[i][j]), pixbufboard[imgtypeboard[i][j]][max(0, f)]);
				else
					gtk_image_set_from_pixbuf(GTK_IMAGE(imageboard[i][j]), pixbufboard[0][max(1, f)]);
			}
			else
			{
				int f = 0, _f = 0;
				int x, y, z = 0;
				if(movepath[piecenum-1]/boardsizew == i && movepath[piecenum-1]%boardsizew == j) f = 2;
				if(refreshboardflag == 1 && movepath[piecenum-2]/boardsizew == i && movepath[piecenum-2]%boardsizew == j) _f = 2;
				if(refreshboardflag == 1 && f) z = 1;
				if(shownumber)
				{
					y = imgtypeboard[i][j]%9;
					x = 3+board[i][j]-1-z;
					if(pixbufboardnumber[y][x][boardnumber[i][j]-z][(f || _f)?1:0] == NULL)
					{
						char n[10];
						sprintf(n, "%d", boardnumber[i][j]-z);
						if(f || _f)
						{
							pixbufboardnumber[y][x][boardnumber[i][j]-z][1] = draw_overlay(pixbufboard[y][x], gdk_pixbuf_get_width(pixbufboard[y][x]), gdk_pixbuf_get_height(pixbufboard[y][x]), n, "#FF0000", showsmallfont);
						}
						else
						{
							if(boardnumber[i][j] % 2 == 1)
								pixbufboardnumber[y][x][boardnumber[i][j]-z][0] = draw_overlay(pixbufboard[y][x], gdk_pixbuf_get_width(pixbufboard[y][x]), gdk_pixbuf_get_height(pixbufboard[y][x]), n, "#FFFFFF", showsmallfont);
							else
								pixbufboardnumber[y][x][boardnumber[i][j]-z][0] = draw_overlay(pixbufboard[y][x], gdk_pixbuf_get_width(pixbufboard[y][x]), gdk_pixbuf_get_height(pixbufboard[y][x]), n, "#000000", showsmallfont);
						}
					}
					gtk_image_set_from_pixbuf(GTK_IMAGE(imageboard[i][j]), pixbufboardnumber[y][x][boardnumber[i][j]-z][(f || _f)?1:0]);
				}
				else
				{
					y = imgtypeboard[i][j]%9;
					x = 3+f+_f+board[i][j]-1-z;
					gtk_image_set_from_pixbuf(GTK_IMAGE(imageboard[i][j]), pixbufboard[y][x]);
				}
			}
		}
	}
	/*
	//for debug
	for(i=0; i<boardsizeh; i++)
	{
		for(j=0; j<boardsizew; j++)
		{
			if(i <= 8 && j <= 10)
			{
				gtk_image_set_from_pixbuf(GTK_IMAGE(imageboard[i][j]), pixbufboard[i][j]);
			}
		}
	}
	*/
}
int is_legal_move(int y, int x)
{
	return y>=0 && x>=0 && y<boardsizeh && x<boardsizew && board[y][x] == 0;
}
void make_move(int y, int x)
{
	int i, j, k;

	board[y][x] = piecenum%2+1;
	boardnumber[y][x] = piecenum+1;
	if(movepath[piecenum] != y*boardsizew+x)
	{
		movepath[piecenum] = y*boardsizew+x;
		for(i=piecenum+1; i<MAX_SIZE*MAX_SIZE; i++) movepath[i] = -1;
	}

	piecenum ++;
	if(piecenum == boardsizeh*boardsizew) isgameover = 1;

	memset(bestline, 0, sizeof(bestline));

	memset(boardbest, 0, sizeof(boardbest));
	memset(boardlose, 0, sizeof(boardlose));
	memset(boardpos, 0, sizeof(boardpos));
	
	refresh_board();
	for(i=0; i<8; i+=2)
	{
		int ny, nx;
		k = 1;
		ny = y;
		nx = x;
		for(j=1; j<6; j++)
		{
			ny += movy[i];
			nx += movx[i];
			if(nx<0 || ny<0 || nx>=boardsizew || ny>=boardsizeh) break;
			if(board[ny][nx] != board[y][x]) break;
			k ++;
		}
		ny = y;
		nx = x;
		for(j=1; j<6; j++)
		{
			ny -= movy[i];
			nx -= movx[i];
			if(nx<0 || ny<0 || nx>=boardsizew || ny>=boardsizeh) break;
			if(board[ny][nx] != board[y][x]) break;
			k ++;
		}
		if(k==5 || (k>5 && inforule != 1))
		{
			isgameover = 1;
			break;
		}
	}
}
void show_forbid()
{
	int i;
	char command[80];
	if((computerside&1) || (piecenum%2))
	{
		memset(forbid, 0, sizeof(forbid));
		return;
	}
	sprintf(command, "start %d %d\n", boardsizew, boardsizeh);
	send_command(command);
	sprintf(command, "yxboard\n");
	send_command(command);
	for(i=0; i<piecenum; i++)
	{
		sprintf(command, "%d,%d,%d\n", movepath[i]/boardsizew,
			movepath[i]%boardsizew, piecenum%2==i%2 ? 1 : 2);
		send_command(command);
	}
	sprintf(command, "done\n");
	send_command(command);
	sprintf(command, "yxshowforbid\n");
	send_command(command);
}
void openbook_enlarge()
{
	BOOK *newbook;
	int i;
	if (openbooknum + 1 < openbooksize) return;
	if (openbooksize == 0) openbooksize = 256;
	openbooksize *= 2;
	newbook = (BOOK *)malloc(openbooksize * sizeof(BOOK));
	if (openbook != NULL)
	{
		for (i = 0; i < openbooknum; i++)
		{
			newbook[i] = openbook[i];
		}
		free(openbook);
	}
	openbook = newbook;
}
int search_openbook(I64 zobkey)
{
	int l, r, m;
	static int flag = -1;
	if (boardsizeh > 20 || boardsizew > 20) return 0; //TODO: support board of size > 20
	if(flag != inforule+(specialrule<<2))
	{
		char name[80];
		FILE *in;
		openbooknum = 0;
		if(specialrule == 2)
		{
			sprintf(name, "book3_%d_%d.dat", boardsizeh, boardsizew);
		}
		else if(specialrule == 1)
		{
			sprintf(name, "book4_%d_%d.dat", boardsizeh, boardsizew);
		}
		else
		{
			sprintf(name, "book%d_%d_%d.dat", inforule, boardsizeh, boardsizew);
		}
		//printf_log("Loading openbook \"%s\"...", name);
		if((in = fopen(name, "rb")) != NULL)
		{
			while(!feof(in))
			{
				openbook_enlarge();
				fread(&openbook[openbooknum].zobkey, sizeof(I64), 1, in);
				fread(&openbook[openbooknum].status, sizeof(char), 1, in);
				openbooknum ++;
			}
			fclose(in);
			//printf_log("Done (%d entries)\n", openbooknum);
		}
		else
		{
			//printf_log("Error\n");
		}
		flag = inforule+(specialrule<<2);
	}
	if(openbooknum == 0) return 0;
	l = 0;
	r = openbooknum-1;
	while(l <= r)
	{
		m = (l+r)/2;
		if(zobkey >= openbook[m].zobkey)
			l = m + 1;
		else
			r = m - 1;
	}
	if(l>0 && openbook[l-1].zobkey == zobkey)
	{
		return openbook[l-1].status;
	}
	return 0;
}

int moveopenbookexclude[MAX_SIZE][MAX_SIZE] = {{0}};

int move_openbook(int *besty, int *bestx)
{
	int i, j, k;
	int p;
	int result[MAX_SIZE][MAX_SIZE] = {{0}};
	int resultnum = 0;
	int bestv = 0;

	if(zobristflag)
	{
		FILE *in;
		if((in = fopen("base.dat", "r")) != NULL)
		{
			//TODO: support board of size > 20
			for(i=0; i<20; i++)
			{
				for(j=0; j<20; j++)
				{
					for(k=0; k<3; k++)
					{
						fscanf(in, "%I64d ", &zobrist[i][j][k]);
					}
				}
			}
			fclose(in);
			zobristflag = 0;
		}
		else
		{
			//printf_log("base.dat is lost\n");
			return 0;
		}
	}
	if(bestx==NULL && besty==NULL) return 0; //just for init zobrist

	for(k=0; k<8; k++)
	{
		int _x, _y;
		I64 zobkey;
		I64 _zobkey;
		_zobkey = 0;
		if (boardsizew != boardsizeh)
		{
			if (k == 0 || k == 2 || k == 4 || k == 6) continue;
		}
		for(i=0; i<piecenum; i++)
		{
			_y = movepath[i]/boardsizew;
			_x = movepath[i]%boardsizew;
			switch(k)
			{
				case 0: _zobkey ^= zobrist[_x][_y][i%2]; break;
				case 1: _zobkey ^= zobrist[_y][_x][i%2]; break;
				case 2: _zobkey ^= zobrist[boardsizeh-1-_x][_y][i%2]; break;
				case 3: _zobkey ^= zobrist[boardsizeh-1-_y][_x][i%2]; break;
				case 4: _zobkey ^= zobrist[_x][boardsizew-1-_y][i%2]; break;
				case 5: _zobkey ^= zobrist[_y][boardsizew-1-_x][i%2]; break;
				case 6: _zobkey ^= zobrist[boardsizeh-1-_x][boardsizew-1-_y][i%2]; break;
				case 7: _zobkey ^= zobrist[boardsizeh-1-_y][boardsizew-1-_x][i%2]; break;
			}
		}
		for(i=0; i<boardsizeh; i++)
		{
			for(j=0; j<boardsizew; j++)
			{
				_y = i;
				_x = j;
				if(moveopenbookexclude[_y][_x]) continue;
				if(board[_y][_x] != 0 || result[_y][_x] != 0) continue;
				zobkey = _zobkey;
				switch(k)
				{
					case 0: zobkey ^= zobrist[_x][_y][piecenum%2]; break;
					case 1: zobkey ^= zobrist[_y][_x][piecenum%2]; break;
					case 2: zobkey ^= zobrist[boardsizeh-1-_x][_y][piecenum%2]; break;
					case 3: zobkey ^= zobrist[boardsizeh-1-_y][_x][piecenum%2]; break;
					case 4: zobkey ^= zobrist[_x][boardsizew-1-_y][piecenum%2]; break;
					case 5: zobkey ^= zobrist[_y][boardsizew-1-_x][piecenum%2]; break;
					case 6: zobkey ^= zobrist[boardsizeh-1-_x][boardsizew-1-_y][piecenum%2]; break;
					case 7: zobkey ^= zobrist[boardsizeh-1-_y][boardsizew-1-_x][piecenum%2]; break;
				}
				if(search_openbook(zobkey) == 'a')
				{
					if(bestv != 'a')
					{
						memset(result, 0, sizeof(result));
						resultnum = 0;
						bestv = 'a';
					}
					result[_y][_x] = 1;
					resultnum ++;
				}
				else if(specialrule == 1)
				{
					if(bestv != 'a' &&
					   ((piecenum%2==0 && search_openbook(zobkey)>='C' && search_openbook(zobkey)<='G' && (search_openbook(zobkey)<bestv || bestv==0)) ||
					    (piecenum%2==1 && search_openbook(zobkey)>='A' && search_openbook(zobkey)<='E' && (search_openbook(zobkey)>bestv || bestv==0))))
					{
						memset(result, 0, sizeof(result));
						resultnum = 0;
						bestv = search_openbook(zobkey);
						result[_y][_x] = 1;
						resultnum ++;
					}
					if((search_openbook(zobkey) == '!') ||
					   (search_openbook(zobkey) >= '1' && search_openbook(zobkey) <= '1') ||
					   (piecenum%2==0 && search_openbook(zobkey)<='B' && search_openbook(zobkey)>='A') ||
					   (piecenum%2==1 && search_openbook(zobkey)>='F' && search_openbook(zobkey)<='G')
					  )
					{
						if(bestv != 'a')
						{
							memset(result, 0, sizeof(result));
							resultnum = 0;
							bestv = 'a';
						}
						result[_y][_x] = 1;
						resultnum ++;
					}
				}
			}
		}
	}

	if(resultnum == 0) return 0;
	p = rand() % resultnum;
	k = 0;
	for(i=0; i<boardsizeh; i++)
	{
		for(j=0; j<boardsizew; j++)
		{
			if(result[i][j])
			{
				if(p == k)
				{
					*besty = i;
					*bestx = j;
					return 1;
				}
				k ++;
			}
		}
	}
	return 1; //in fact this statement will never be reached
}

//this function can generate the best n moves from the openbook
//and store the results to besty and bestx
int move_openbook_n(int n, int *besty, int *bestx, int force)
{
	int i, j, k, l;
	double d[4][4];
	int px[4], py[4];
	int sy[3] = {0};
	/*
	not necessary, since the judgement of board[][] has been done in move_openbook

	if(force != 2)
	{
		for(i=0; i<piecenum; i++)
		{
			moveopenbookexclude[movepath[piecenum-1]/boardsizew][movepath[piecenum-1]%boardsizew] = 1;
		}
	}
	*/
	for(i=0; i<4; i++)
	{
		py[i] = movepath[i] / boardsizew;
		px[i] = movepath[i] % boardsizew;
	}
	for(i=0; i<4; i++)
	{
		for(j=0; j<4; j++)
		{
			d[i][j] = (double)(px[i] - px[j])*(double)(px[i] - px[j]) + (double)(py[i] - py[j])*(double)(py[i] - py[j]);
			d[i][j] = sqrt(d[i][j]);
		}
	}
	if(fabs(d[0][1] - d[0][3]) <= 1e-6 && fabs(d[2][1] - d[2][3]) <= 1e-6) sy[0] = 1;
	if((fabs(d[0][1]+d[2][1]-d[0][2]) <= 1e-6 || fabs(d[0][1]+d[0][2]-d[1][2]) <= 1e-6 || fabs(d[0][2]+d[2][1]-d[0][1]) <= 1e-6) &&
		(fabs(d[0][3]+d[2][3]-d[0][2]) <= 1e-6 || fabs(d[0][3]+d[0][2]-d[3][2]) <= 1e-6 || fabs(d[0][2]+d[2][3]-d[0][3]) <= 1e-6)) sy[0] = 1;
	if(fabs(d[0][1] - d[2][3]) <= 1e-6 && fabs(d[1][2] - d[0][3]) <= 1e-6) sy[1] = 1;
	if(fabs(d[0][1] - d[2][1]) <= 1e-6 && fabs(d[0][3] - d[2][3]) <= 1e-6) sy[2] = 1;
	if((fabs(d[0][1]+d[0][3]-d[1][3]) <= 1e-6 || fabs(d[0][1]+d[1][2]-d[0][3]) <= 1e-6 || fabs(d[0][3]+d[1][3]-d[0][1]) <= 1e-6) &&
		(fabs(d[2][1]+d[2][3]-d[1][3]) <= 1e-6 || fabs(d[1][2]+d[1][3]-d[2][3]) <= 1e-6 || fabs(d[2][3]+d[1][3]-d[2][1]) <= 1e-6)) sy[2] = 1;

	for(i=0; i<n; i++)
	{
		if(move_openbook(besty+i, bestx+i) == 0) break;
		moveopenbookexclude[besty[i]][bestx[i]] = 1;
		if(sy[0] || sy[1] || sy[2])
		{
			double d1, d2, d3, d4;
			d1 = (besty[i] - py[0])*(besty[i] - py[0]) + (bestx[i] - px[0])*(bestx[i] - px[0]);
			d2 = (besty[i] - py[2])*(besty[i] - py[2]) + (bestx[i] - px[2])*(bestx[i] - px[2]);
			d3 = (besty[i] - py[1])*(besty[i] - py[1]) + (bestx[i] - px[1])*(bestx[i] - px[1]);
			d4 = (besty[i] - py[3])*(besty[i] - py[3]) + (bestx[i] - px[3])*(bestx[i] - px[3]);
			
			for(j=0; j<boardsizeh; j++)
			{
				for(k=0; k<boardsizew; k++)
				{
					double _d1, _d2, _d3, _d4;
					if(moveopenbookexclude[j][k]) continue;
					_d1 = (j - py[0])*(j - py[0]) + (k - px[0])*(k - px[0]);
					_d2 = (j - py[2])*(j - py[2]) + (k - px[2])*(k - px[2]);
					_d3 = (j - py[1])*(j - py[1]) + (k - px[0])*(k - px[1]);
					_d4 = (j - py[3])*(j - py[3]) + (k - px[2])*(k - px[3]);
					
					if(sy[0] && fabs(d1 - _d1) <= 1e-6 && fabs(d2 - _d2) <= 1e-6)
					{
						moveopenbookexclude[j][k] = 1;
					}
					if(sy[1] && fabs(d1 - _d2) <= 1e-6 && fabs(d2 - _d1) <= 1e-6)
					{
						moveopenbookexclude[j][k] = 1;
					}
					if(sy[2] && fabs(d3 - _d3) <= 1e-6 && fabs(d4 - _d4) <= 1e-6)
					{
						moveopenbookexclude[j][k] = 1;
					}
				}
			}
		}
	}

	if(force == 1 && i<n)
	{
		int x, y, nx, ny;
		y = movepath[piecenum-1] / boardsizew;
		x = movepath[piecenum-1] % boardsizew;
		for(j=1; j<max(boardsizeh, boardsizew); j++)
		{
			for(k=-j; k<=j; k++)
			{
				int t = j - abs(k);
				for(l=-t; l<=t; l++)
				{
					nx = x + k;
					ny = y + l;
					if(nx < 0 || ny < 0 || nx >= boardsizew || ny >= boardsizeh) continue;
					movepath[piecenum-1] = ny * boardsizew + nx;
					i += move_openbook_n(n-i, besty+i, bestx+i, 2);
					if(i >= n) break;
				}
				if(i >= n) break;
			}
			if(i >= n) break;
		}
		movepath[piecenum-1] = y * boardsizew + x;
		if(i < n)
		{
			printf_log("ERROR Opening book error\n");
			for(; i<n; i++)
			{
				besty[i] = bestx[i] = 0;
			}
		}
	}
	if(force != 2)
	{
		memset(moveopenbookexclude, 0, sizeof(moveopenbookexclude));
	}
	return i;
}

int eval_openbook()
{
	int i, k;
	int v;
	move_openbook(NULL, NULL);
	for(k=0; k<8; k++)
	{
		int _x, _y;
		I64 _zobkey;
		_zobkey = 0;
		if (boardsizew != boardsizeh)
		{
			if (k == 0 || k == 2 || k == 4 || k == 6) continue;
		}
		for(i=0; i<piecenum; i++)
		{
			_y = movepath[i]/boardsizew;
			_x = movepath[i]%boardsizew;
			switch(k)
			{
				case 0: _zobkey ^= zobrist[_x][_y][i%2]; break;
				case 1: _zobkey ^= zobrist[_y][_x][i%2]; break;
				case 2: _zobkey ^= zobrist[boardsizeh-1-_x][_y][i%2]; break;
				case 3: _zobkey ^= zobrist[boardsizeh-1-_y][_x][i%2]; break;
				case 4: _zobkey ^= zobrist[_x][boardsizew-1-_y][i%2]; break;
				case 5: _zobkey ^= zobrist[_y][boardsizew-1-_x][i%2]; break;
				case 6: _zobkey ^= zobrist[boardsizeh-1-_x][boardsizew-1-_y][i%2]; break;
				case 7: _zobkey ^= zobrist[boardsizeh-1-_y][boardsizew-1-_x][i%2]; break;
			}
		}
		v = search_openbook(_zobkey);
		if(v != 0) return v;
	}
	return 0;
}

void show_dialog_undo_warning_query(GtkWidget *window)
{
	GtkWidget *dialog;
	gint result;
	dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, language==0?"The operation will stop the calculation. Do you want to continue?":_T(clanguage[12]));
	gtk_window_set_title(GTK_WINDOW(dialog), "Yixin");
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result)
	{
		case GTK_RESPONSE_YES:
			change_piece(window, (gpointer)1);
			break;
		case GTK_RESPONSE_NO:
			break;
	}
	gtk_widget_destroy(dialog);
}

void show_dialog_swap_query(GtkWidget *window)
{
	GtkWidget *dialog;
	gint result;
	dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, language==0?"Swap?":_T(clanguage[13]));
	gtk_window_set_title(GTK_WINDOW(dialog), "Yixin");
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result)
	{
		case GTK_RESPONSE_YES:
			if(specialrule == 2)
			{
				isneedrestart = 1;
				make_move(4, 5);
				if(computerside == 2)
				{
					change_side_menu(1, NULL);
					change_side_menu(-2, NULL);
				}
				else
				{
					change_side_menu(-1, NULL);
					change_side_menu(2, NULL);
				}
			}
			else if(specialrule == 1)
			{
				isneedrestart = 1;
				//TODO: should support more variations (with openbook)
				make_move(boardsizeh/2-1, boardsizew/2+1);
				if(computerside == 2)
				{
					change_side_menu(1, NULL);
					change_side_menu(-2, NULL);
				}
				else
				{
					change_side_menu(-1, NULL);
					change_side_menu(2, NULL);
				}
			}
			if(language) printf_log(clanguage[14]); else printf_log("Swap");
			printf_log("\n");
			break;
		case GTK_RESPONSE_NO:
			printf_log("\n");
			break;
	}
	gtk_widget_destroy(dialog);
}

void show_dialog_swap_info(GtkWidget *window)
{
	GtkWidget *dialog;
	gint result;
	dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO, GTK_BUTTONS_OK, language==0?"Swap":_T(clanguage[15]));
	gtk_window_set_title(GTK_WINDOW(dialog), "Yixin");
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	isneedrestart = 1;
	if(computerside == 2)
	{
		change_side_menu(1, NULL);
		change_side_menu(-2, NULL);
	}
	else
	{
		change_side_menu(-1, NULL);
		change_side_menu(2, NULL);
	}
	gtk_widget_destroy(dialog);
}

void show_dialog_illegal_opening(GtkWidget *window)
{
	GtkWidget *dialog;
	gint result;
	dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO, GTK_BUTTONS_OK, language==0?"Illegal Opening": _T(clanguage[16]));
	gtk_window_set_title(GTK_WINDOW(dialog), "Yixin");
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	isneedrestart = 1;
	gtk_widget_destroy(dialog);
}

gboolean on_button_press_windowmain(GtkWidget *widget, GdkEventButton *event, GdkWindowEdge edge)
{
	int x, y;
	int size;
	char command[80];
	if(!isthinking && event->type == GDK_BUTTON_PRESS)
	{
		if(event->button == 1)
		{
			size = gdk_pixbuf_get_width(pixbufboard[0][0]);
			if(event->x - imageboard[0][0]->allocation.x < 0 || event->y - imageboard[0][0]->allocation.y < 0)
			{
				x = y = -1;
			}
			else
			{
				x = (int)((event->x - imageboard[0][0]->allocation.x)/size);
				y = (int)((event->y - imageboard[0][0]->allocation.y)/size);
			}
			if(x>=0 && x<boardsizew && y>=0 && y<boardsizeh && !isgameover)
			{
				if(!refreshboardflag && (specialrule!=1 || piecenum>=3 || piecenum==0) && (((computerside&1)&&piecenum%2==0) || ((computerside&2)&&piecenum%2==1)))
				{
					int i;
					//the first move of `swap after 1st move' rule
					if(specialrule == 2 && piecenum == 0 && computerside != 3)
					{
						isneedrestart = 1;
						make_move(2, 3);
						show_dialog_swap_query(widget);
					}
					else if(specialrule == 1 && piecenum == 0 && computerside != 3)
					{
						isneedrestart = 1;
						//TODO: it should support more variations (with openbook)
						make_move(boardsizeh/2, boardsizew/2);
						make_move(boardsizeh/2-1, boardsizew/2);
						make_move(boardsizeh/2-2, boardsizew/2+2);
						show_dialog_swap_query(widget);
					}
					else if(specialrule == 1 && piecenum == 4 && computerside != 3)
					{
						int besty[2], bestx[2];
						isneedrestart = 1;
						if(movepath[0]%boardsizew == boardsizew/2 && movepath[0]/boardsizew == boardsizeh/2 &&
							movepath[1]%boardsizew <= boardsizew/2+1 && movepath[1]%boardsizew >= boardsizew/2-1 &&
							movepath[1]/boardsizew <= boardsizeh/2+1 && movepath[1]/boardsizew >= boardsizeh/2-1 &&
							movepath[2]%boardsizew <= boardsizew/2+2 && movepath[2]%boardsizew >= boardsizew/2-2 &&
							movepath[2]/boardsizew <= boardsizeh/2+2 && movepath[2]/boardsizew >= boardsizeh/2-2)
						{
							//should use openbook
							refreshboardflag = 1;
							move_openbook_n(2, besty, bestx, 1);
							make_move(besty[0], bestx[0]);
							make_move(besty[1], bestx[1]);
						}
						else
						{
							show_dialog_illegal_opening(widget);
							new_game(NULL, NULL);
						}
					}
					else if(useopenbook && move_openbook(&y, &x) && is_legal_move(y, x))
					{
						isneedrestart = 1;
						if(language == 0)
						{
							printf_log("Use openbook");
						}
						else
						{
							printf_log(clanguage[17]);
						}
						printf_log("\n");
						make_move(y, x);
						if(inforule == 2 && (computerside&1)==0)
						{
							sprintf(command, "yxshowforbid\n");
							send_command(command);
						}
					}
					else
					{
						isthinking = 1;
						isneedrestart = 0;
						timestart = clock();
						sprintf(command, "INFO time_left %d\n", timeoutmatch-timeused);
						send_command(command);
						if (hashautoclear) send_command("yxhashclear\n");
						sprintf(command, "start %d %d\n", boardsizew, boardsizeh);
						send_command(command);
						sprintf(command, "board\n");
						send_command(command);
						for(i=0; i<piecenum; i++)
						{
							sprintf(command, "%d,%d,%d\n", movepath[i]/boardsizew,
								movepath[i]%boardsizew, piecenum%2==i%2 ? 1 : 2);
							send_command(command);
						}
						sprintf(command, "done\n");
						send_command(command);
					}
				}
				else
				{
					if(refreshboardflag == 1 && computerside != 2)
					{
						if(boardnumber[y][x] == piecenum || boardnumber[y][x] == piecenum-1)
						{
							isneedrestart = 1;
							refreshboardflag = 0;
							if(boardnumber[y][x] == piecenum)
							{
								change_piece(NULL, (gpointer)1);
								change_piece(NULL, (gpointer)1);
								make_move(y, x);
							}
							else
							{
								change_piece(NULL, (gpointer)1);
							}
						}
					}
					else if(board[y][x] == 0 && (piecenum%2==1 || !forbid[y][x]))
					{
						int i;
						int flag = 0;
						
						make_move(y, x);
						if(specialrule == 2 && piecenum == 1 && computerside != 0)
						{
							int _x, _y;
							_x = min(x, boardsizew-1-x);
							_y = min(y, boardsizeh-1-y);
							//the condition for swapping under `swap after 1st move' rule
							if((((_x==2&&_y==3)||(_x==3&&_y==2))&&rand()%2==1) || (_x>1 && _y>1 && _x+_y>5))
							{
								show_dialog_swap_info(widget);
							}
						}
						if(specialrule == 1 && piecenum == 3 && /*computerside != 0 &&*/ computerside != 1)
						{
							//check whether the current opening is one of the 26 standard openings
							if(movepath[0]%boardsizew == boardsizew/2 && movepath[0]/boardsizew == boardsizeh/2 &&
								movepath[1]%boardsizew <= boardsizew/2+1 && movepath[1]%boardsizew >= boardsizew/2-1 &&
								movepath[1]/boardsizew <= boardsizeh/2+1 && movepath[1]/boardsizew >= boardsizeh/2-1 &&
								movepath[2]%boardsizew <= boardsizew/2+2 && movepath[2]%boardsizew >= boardsizew/2-2 &&
								movepath[2]/boardsizew <= boardsizeh/2+2 && movepath[2]/boardsizew >= boardsizeh/2-2)
							{
								//do nothing
							}
							else
							{
								show_dialog_illegal_opening(widget);
								new_game(NULL, NULL);
								flag = 1;
							}
						}
						if(specialrule == 1 && piecenum == 3 && computerside != 0 && computerside != 1)
						{
							//TODO: should be decided by openbook
							if(eval_openbook() <= 'C' && eval_openbook() >= 'A')
							{
								show_dialog_swap_info(widget);
								flag = 1;
							}
						}
						if(specialrule == 1 && piecenum == 4 && computerside != 0 && computerside != 2)
						{
							int besty[2], bestx[2];
							isneedrestart = 1;
							
							if(movepath[0]%boardsizew == boardsizew/2 && movepath[0]/boardsizew == boardsizeh/2 &&
								movepath[1]%boardsizew <= boardsizew/2+1 && movepath[1]%boardsizew >= boardsizew/2-1 &&
								movepath[1]/boardsizew <= boardsizeh/2+1 && movepath[1]/boardsizew >= boardsizeh/2-1 &&
								movepath[2]%boardsizew <= boardsizew/2+2 && movepath[2]%boardsizew >= boardsizew/2-2 &&
								movepath[2]/boardsizew <= boardsizeh/2+2 && movepath[2]/boardsizew >= boardsizeh/2-2)
							{
								refreshboardflag = 1;
								//use openbook
								move_openbook_n(2, besty, bestx, 1);
								make_move(besty[0], bestx[0]);
								make_move(besty[1], bestx[1]);
							}
							else
							{
								show_dialog_illegal_opening(widget);
								new_game(NULL, NULL);
							}
							flag = 1;
						}
						if(specialrule == 1 && piecenum == 5 && computerside != 0 && computerside != 1)
						{
							refreshboardflag = 1;
							flag = 1;
						}
						if(specialrule == 1 && piecenum == 6 && computerside != 0 && computerside != 1)
						{
							int x1, y1;
							int x2, y2;
							int v1, v2;
							refreshboardflag = 0;
							isneedrestart = 1;
							if(movepath[0]%boardsizew == boardsizew/2 && movepath[0]/boardsizew == boardsizeh/2 &&
								movepath[1]%boardsizew <= boardsizew/2+1 && movepath[1]%boardsizew >= boardsizew/2-1 &&
								movepath[1]/boardsizew <= boardsizeh/2+1 && movepath[1]/boardsizew >= boardsizeh/2-1 &&
								movepath[2]%boardsizew <= boardsizew/2+2 && movepath[2]%boardsizew >= boardsizew/2-2 &&
								movepath[2]/boardsizew <= boardsizeh/2+2 && movepath[2]/boardsizew >= boardsizeh/2-2)
							{
								//use openbook (should analyze at the same time)
								y1 = movepath[piecenum-1] / boardsizew;
								x1 = movepath[piecenum-1] % boardsizew;
								y2 = movepath[piecenum-2] / boardsizew;
								x2 = movepath[piecenum-2] % boardsizew;
								change_piece(NULL, (gpointer)1);
								v2 = eval_openbook();
								change_piece(NULL, (gpointer)1);
								make_move(y1, x1);
								v1 = eval_openbook();
								if(v1==0 || v2=='!' || v2=='1' || v2=='a' ||
								   (v2>='A' && v2<='F' && v1>='A' && v1<='F' && v2<=v1) ||
								   (v2>='1' && v2<='9' && v1>='1' && v1<='9' && v2<=v1) ||
								   (v2>='1' && v2<='9' && (v1<'1' || v1>'9')))
								{
									//do nothing
								}
								else
								{
									change_piece(NULL, (gpointer)1);
									make_move(y2, x2);
								}
							}
							else
							{
								show_dialog_illegal_opening(widget);
								new_game(NULL, NULL);
							}
						}
						show_forbid();
						if(!isgameover && !flag &&  (specialrule!=1 || piecenum>=3) && (((computerside&1)&&piecenum%2==0) || ((computerside&2)&&piecenum%2==1)))
						{
							if(useopenbook && move_openbook(&y, &x) && is_legal_move(y, x))
							{
								isneedrestart = 1;
								if (language) printf_log(clanguage[17]); else printf_log("Use openbook");
								printf_log("\n");
								make_move(y, x);
								if(inforule == 2 && (computerside&1)==0)
								{
									sprintf(command, "yxshowforbid\n");
									send_command(command);
								}
							}
							else
							{
								if(isneedrestart)
								{
									isthinking = 1;
									isneedrestart = 0;
									timeused = 0;
									timestart = clock();
									sprintf(command, "INFO time_left %d\n", timeoutmatch-timeused);
									send_command(command);
									if (hashautoclear) send_command("yxhashclear\n");
									sprintf(command, "start %d %d\n", boardsizew, boardsizeh);
									send_command(command);
									sprintf(command, "board\n");
									send_command(command);
									for(i=0; i<piecenum; i++)
									{
										sprintf(command, "%d,%d,%d\n", movepath[i]/boardsizew,
											movepath[i]%boardsizew, piecenum%2==i%2 ? 1 : 2);
										send_command(command);
									}
									sprintf(command, "done\n");
									send_command(command);
								}
								else
								{
									sprintf(command, "INFO time_left %d\n", timeoutmatch-timeused);
									send_command(command);
									if (hashautoclear) send_command("yxhashclear\n");
									sprintf(command, "turn %d,%d\n", y, x);
									send_command(command);
									isthinking = 1;
									timestart = clock();
								}
							}
						}
					}
				}
				return FALSE;
			}
		}
	}
	if(event->type == GDK_BUTTON_PRESS && event->button == 3)
	{
		if(showwarning && isthinking)
			show_dialog_undo_warning_query(widget);
		else
			change_piece(widget, (gpointer)1);
	}
	return FALSE; /* why FALSE? */
}

int is_integer(const char *str)
{
	while(*str)
	{
		if(*str < '0' || *str > '9') return 0;
		str ++;
	}
	return 1;
}

void set_level(int x)
{
	gchar command[80];
	levelchoice = x;
	if(levelchoice == 4)
	{
		sprintf(command, "INFO timeout_turn %d\n", timeoutturn);
		send_command(command);
		sprintf(command, "INFO timeout_match %d\n", timeoutmatch);
		send_command(command);
		sprintf(command, "INFO time_left %d\n", timeoutmatch-timeused);
		send_command(command);
		sprintf(command, "INFO max_node %d\n", maxnode); //now it should not be -1
		send_command(command);
		sprintf(command, "INFO max_depth %d\n", maxdepth);
		send_command(command);
	}
	else
	{
		switch(levelchoice)
		{
			case 0:
				sprintf(command, "INFO max_node %d\n", 60000);
				send_command(command);
				break;
			case 1:
				sprintf(command, "INFO max_node %d\n", 30000); //if the speed is 500k, then it will take at most 60s
				send_command(command);
				break;
			case 2:
				sprintf(command, "INFO max_node %d\n", 20000);
				send_command(command);
				break;
			case 3:
				sprintf(command, "INFO max_node %d\n", 10000);
				send_command(command);
				break;
			case 5:
				sprintf(command, "INFO max_node %d\n", 240000);
				send_command(command);
				break;
			case 6:
				sprintf(command, "INFO max_node %d\n", 1920000);
				send_command(command);
				break;
			case 7:
				sprintf(command, "INFO max_node %d\n", 500000000);
				send_command(command);
				break;
			case 8:
				sprintf(command, "INFO max_node %d\n", -1);
				send_command(command);
				break;
		}
		timeoutmatch = 1000000000;
		timeoutturn = 2000000;
		sprintf(command, "INFO timeout_match %d\n", timeoutmatch);
		send_command(command);
		sprintf(command, "INFO time_left %d\n", timeoutmatch);
		send_command(command);
		sprintf(command, "INFO timeout_turn %d\n", timeoutturn);
		send_command(command);
		sprintf(command, "INFO max_depth %d\n", boardsizeh * boardsizew);
		send_command(command);
	}
}

void set_cautionfactor(int x)
{
	gchar command[80];
	if(x < 0) x = 0;
	if(x > CAUTION_NUM) x = CAUTION_NUM;
	cautionfactor = x;
	sprintf(command, "INFO caution_factor %d\n", cautionfactor);
	send_command(command);
}

void set_threadnum(int x)
{
	gchar command[80];
	if(x < 1) x = 1;
	//if(x > maxthreadnum) x = maxthreadnum;
	threadnum = x;
	sprintf(command, "INFO thread_num %d\n", threadnum);
	send_command(command);
}

void set_threadsplitdepth(int x)
{
	gchar command[80];
	if(x < MIN_SPLIT_DEPTH) x = MIN_SPLIT_DEPTH;
	if(x > MAX_SPLIT_DEPTH) x = MAX_SPLIT_DEPTH;
	threadsplitdepth = x;
	sprintf(command, "INFO thread_split_depth %d\n", threadsplitdepth);
	send_command(command);
}

void set_hashsize(int x)
{
	gchar command[80];
	if(x < 0) x = 0;
	if(x > maxhashsize) x = maxhashsize;
	hashsize = x;
	sprintf(command, "INFO hash_size %d\n", hashsize==0 ? 0 : (1<<hashsize));
	send_command(command);
}

void set_pondering(int x)
{
	gchar command[80];
	if (x < 0) x = 0;
	if (x > 1) x = 1;
	infopondering = x;
	sprintf(command, "INFO pondering %d\n", infopondering);
	send_command(command);
}

void set_checkmate(int x)
{
	gchar command[80];
	if (x < 0) x = 0;
	if (x > 2) x = 2;
	infocheckmate = x;
	sprintf(command, "INFO checkmate %d\n", infocheckmate);
	send_command(command);
}

void show_dialog_settings_custom_entry(GtkWidget *widget, gpointer data)
{
	static GtkWidget *editable[4];
	static int flag = 0;
	int i;
	if(widget == NULL)
	{
		if(data == 0)
		{
			flag = 0;
		}
		else
		{
			editable[flag] = (GtkWidget *)data;
			flag ++;
		}
		return;
	}
	for(i=0; i<flag; i++)
	{
		//gtk_editable_set_editable(GTK_EDITABLE(editable[i]), FALSE);
		gtk_widget_set_sensitive(editable[i], (int)data==0 ? FALSE : TRUE);
	}
}

void show_dialog_settings(GtkWidget *widget, gpointer data)
{
	int i;
	//gchar command[80];
	gchar text[80];
	const gchar *ptext;
	GtkWidget *dialog;
	GtkWidget *notebook;
	GtkWidget *notebookvbox[3];
	GtkWidget *hbox[11];
	GtkWidget *radiolevel[9];
	GtkWidget *radiocheckmate[3];
	GtkWidget *labeltimeturn[2], *labeltimematch[2], *labelmaxdepth[2], *labelmaxnode[2], *labelblank[6];
	GtkWidget *entrytimeturn, *entrytimematch, *entrymaxdepth, *entrymaxnode;
	GtkWidget *scalecaution, *scalethreads, *scalesplitdepth, *scalehash;
	GtkWidget *tablesetting;
	gint result;

	show_dialog_settings_custom_entry(NULL, 0);

	dialog = gtk_dialog_new_with_buttons("Settings", data, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, "OK", 1, "Cancel", 2, NULL);
	gtk_window_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

	notebook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), notebook, FALSE, FALSE, 3);
	notebookvbox[0] = gtk_vbox_new(FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), notebookvbox[0], gtk_label_new(language==0?"Level":_T(clanguage[18])));
	notebookvbox[1] = gtk_vbox_new(FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), notebookvbox[1], gtk_label_new(language==0?"Style":_T(clanguage[19])));
	notebookvbox[2] = gtk_vbox_new(FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), notebookvbox[2], gtk_label_new(language==0?"Resource":_T(clanguage[20])));

	for(i=0; i<6; i++)
	{
		labelblank[i] = gtk_label_new(" ");
		gtk_label_set_width_chars(GTK_LABEL(labelblank[i]), 6);
	}

	labeltimeturn[0] = gtk_label_new(language==0?"Turn time:":_T(clanguage[21]));
	entrytimeturn = gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(entrytimeturn), 6);
	sprintf(text, "%d", timeoutturn/1000);
	gtk_entry_set_text(GTK_ENTRY(entrytimeturn), text);
	labeltimeturn[1] = gtk_label_new(language==0?"s":_T(clanguage[22]));
	gtk_misc_set_alignment(GTK_MISC(labeltimeturn[0]), 1, 0.5);
	gtk_misc_set_alignment(GTK_MISC(labeltimeturn[1]), 0, 0.5);

	labeltimematch[0] = gtk_label_new(language==0?"Match time:":_T(clanguage[23]));
	entrytimematch = gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(entrytimematch), 6);
	sprintf(text, "%d", timeoutmatch/1000);
	gtk_entry_set_text(GTK_ENTRY(entrytimematch), text);
	labeltimematch[1] = gtk_label_new(language==0?"s":_T(clanguage[22]));
	gtk_misc_set_alignment(GTK_MISC(labeltimematch[0]), 1, 0.5);
	gtk_misc_set_alignment(GTK_MISC(labeltimematch[1]), 0, 0.5);

	labelmaxdepth[0] = gtk_label_new(language==0?"Max depth:":_T(clanguage[24]));
	entrymaxdepth = gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(entrymaxdepth), 3);
	sprintf(text, "%d", maxdepth);
	gtk_entry_set_text(GTK_ENTRY(entrymaxdepth), text);
	labelmaxdepth[1] = gtk_label_new(language==0?"ply":_T(clanguage[25]));
	gtk_misc_set_alignment(GTK_MISC(labelmaxdepth[0]), 1, 0.5);
	gtk_misc_set_alignment(GTK_MISC(labelmaxdepth[1]), 0, 0.5);

	labelmaxnode[0] = gtk_label_new(language==0?"Max node number:":_T(clanguage[26]));
	entrymaxnode = gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(entrymaxnode), 6);
	sprintf(text, "%d", maxnode/1000);
	gtk_entry_set_text(GTK_ENTRY(entrymaxnode), text);
	labelmaxnode[1] = gtk_label_new(language==0?"M":_T(clanguage[27]));
	gtk_misc_set_alignment(GTK_MISC(labelmaxnode[0]), 1, 0.5);
	gtk_misc_set_alignment(GTK_MISC(labelmaxnode[1]), 0, 0.5);

	show_dialog_settings_custom_entry(NULL, (gpointer)entrytimeturn);
	show_dialog_settings_custom_entry(NULL, (gpointer)entrytimematch);
	show_dialog_settings_custom_entry(NULL, (gpointer)entrymaxdepth);
	show_dialog_settings_custom_entry(NULL, (gpointer)entrymaxnode);

	radiolevel[0] = gtk_radio_button_new_with_label(NULL, language==0?"4 dan":_T(clanguage[28]));
	radiolevel[1] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radiolevel[0])), language==0?"3 dan":_T(clanguage[29]));
	radiolevel[2] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radiolevel[1])), language==0?"2 dan":_T(clanguage[30]));
	radiolevel[3] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radiolevel[2])), language==0?"1 dan":_T(clanguage[31]));
	radiolevel[4] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radiolevel[3])), language==0?"Custom Level":_T(clanguage[32]));
	radiolevel[5] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radiolevel[4])), language==0?"6 dan (Slow)":_T(clanguage[33]));
	radiolevel[6] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radiolevel[5])), language==0?"9 dan (Very Slow)":_T(clanguage[34]));
	radiolevel[7] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radiolevel[6])), language==0?"Meijin (Extremely Slow)":_T(clanguage[35]));
	radiolevel[8] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radiolevel[7])), language==0?"Unlimited Time":_T(clanguage[36]));

	g_signal_connect(G_OBJECT(radiolevel[0]), "toggled", G_CALLBACK(show_dialog_settings_custom_entry), (gpointer)0);
	g_signal_connect(G_OBJECT(radiolevel[1]), "toggled", G_CALLBACK(show_dialog_settings_custom_entry), (gpointer)0);
	g_signal_connect(G_OBJECT(radiolevel[2]), "toggled", G_CALLBACK(show_dialog_settings_custom_entry), (gpointer)0);
	g_signal_connect(G_OBJECT(radiolevel[3]), "toggled", G_CALLBACK(show_dialog_settings_custom_entry), (gpointer)0);
	g_signal_connect(G_OBJECT(radiolevel[4]), "toggled", G_CALLBACK(show_dialog_settings_custom_entry), (gpointer)1);
	g_signal_connect(G_OBJECT(radiolevel[5]), "toggled", G_CALLBACK(show_dialog_settings_custom_entry), (gpointer)0);
	g_signal_connect(G_OBJECT(radiolevel[6]), "toggled", G_CALLBACK(show_dialog_settings_custom_entry), (gpointer)0);
	g_signal_connect(G_OBJECT(radiolevel[7]), "toggled", G_CALLBACK(show_dialog_settings_custom_entry), (gpointer)0);
	g_signal_connect(G_OBJECT(radiolevel[8]), "toggled", G_CALLBACK(show_dialog_settings_custom_entry), (gpointer)0);

	if(levelchoice != 4) show_dialog_settings_custom_entry(widget, (gpointer)0);
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiolevel[levelchoice]), TRUE);

	tablesetting = gtk_table_new(2, 9, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(tablesetting), 0); /* set the row distance between elements to be 0 */
	gtk_table_set_col_spacings(GTK_TABLE(tablesetting), 0); /* set the column distance between elements to be 0 */
	gtk_table_attach_defaults(GTK_TABLE(tablesetting), labelblank[0], 0, 1, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(tablesetting), labelblank[1], 4, 5, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(tablesetting), labelblank[2], 8, 9, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(tablesetting), labeltimeturn[0], 1, 2, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(tablesetting), entrytimeturn, 2, 3, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(tablesetting), labeltimeturn[1], 3, 4, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(tablesetting), labeltimematch[0], 5, 6, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(tablesetting), entrytimematch, 6, 7, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(tablesetting), labeltimematch[1], 7, 8, 0, 1);

	gtk_table_attach_defaults(GTK_TABLE(tablesetting), labelblank[3], 0, 1, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(tablesetting), labelblank[4], 4, 5, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(tablesetting), labelblank[5], 8, 9, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(tablesetting), labelmaxdepth[0], 1, 2, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(tablesetting), entrymaxdepth, 2, 3, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(tablesetting), labelmaxdepth[1], 3, 4, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(tablesetting), labelmaxnode[0], 5, 6, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(tablesetting), entrymaxnode, 6, 7, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(tablesetting), labelmaxnode[1], 7, 8, 1, 2);

	gtk_box_pack_start(GTK_BOX(notebookvbox[0]), radiolevel[4], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[0]), tablesetting, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[0]), radiolevel[7], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[0]), radiolevel[6], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[0]), radiolevel[5], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[0]), radiolevel[0], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[0]), radiolevel[1], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[0]), radiolevel[2], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[0]), radiolevel[3], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[0]), radiolevel[8], FALSE, FALSE, 3);

	scalecaution = gtk_hscale_new_with_range(0, CAUTION_NUM, 1);
	//gtk_scale_set_value_pos(GTK_SCALE(scalecaution), GTK_POS_LEFT);
	gtk_range_set_value(GTK_RANGE(scalecaution), cautionfactor);
	gtk_widget_set_size_request(scalecaution, 100, -1);

	scalethreads = gtk_hscale_new_with_range(1, maxthreadnum, 1);
	gtk_range_set_value(GTK_RANGE(scalethreads), threadnum);
	gtk_widget_set_size_request(scalethreads, 100, -1);

	scalesplitdepth = gtk_hscale_new_with_range(MIN_SPLIT_DEPTH, MAX_SPLIT_DEPTH, 1);
	gtk_range_set_value(GTK_RANGE(scalesplitdepth), threadsplitdepth);
	gtk_widget_set_size_request(scalesplitdepth, 100, -1);

	scalehash = gtk_hscale_new_with_range(0, maxhashsize, 1);
	gtk_range_set_value(GTK_RANGE(scalehash), hashsize);
	gtk_widget_set_size_request(scalehash, 100, -1);

	radiocheckmate[0] = gtk_radio_button_new_with_label(NULL, language == 0 ? "Global Search" : _T(clanguage[87]));
	radiocheckmate[1] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radiocheckmate[0])), language == 0 ? "VCT Search Only" : _T(clanguage[86]));
	radiocheckmate[2] = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(radiocheckmate[1])), language == 0 ? "VC2 Search Only" : _T(clanguage[88]));

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radiocheckmate[infocheckmate]), TRUE);

	hbox[8] = radiocheckmate[0];
	hbox[9] = radiocheckmate[1];
	hbox[10] = radiocheckmate[2];

	hbox[7] = gtk_hseparator_new();

	hbox[1] = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox[1]), gtk_label_new(language==0?"Rash":_T(clanguage[37])), FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(hbox[1]), scalecaution, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(hbox[1]), gtk_label_new(language==0?"Cautious":_T(clanguage[38])), FALSE, FALSE, 3);

	gtk_box_pack_start(GTK_BOX(notebookvbox[1]), hbox[8], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[1]), hbox[9], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[1]), hbox[10], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[1]), hbox[7], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[1]), hbox[1], FALSE, FALSE, 3);

	if (maxthreadnum > 1)
	{
		hbox[2] = gtk_hbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hbox[2]), gtk_label_new(language == 0 ? "Number of Threads" : _T(clanguage[39])), FALSE, FALSE, 3);
		gtk_box_pack_start(GTK_BOX(hbox[2]), scalethreads, FALSE, FALSE, 3);
		hbox[4] = gtk_hbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hbox[4]), gtk_label_new(language == 0 ? "Split Depth" : _T(clanguage[40])), FALSE, FALSE, 3);
		gtk_box_pack_start(GTK_BOX(hbox[4]), scalesplitdepth, FALSE, FALSE, 3);
	}

	hbox[3] = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox[3]), gtk_label_new(language==0?"Hash Size": _T(clanguage[41])), FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(hbox[3]), scalehash, FALSE, FALSE, 3);

	hbox[5] = gtk_check_button_new_with_label(language == 0 ? "Pondering" : _T(clanguage[85]));
	gtk_toggle_button_set_active(hbox[5], infopondering ? TRUE : FALSE);

	hbox[6] = gtk_hseparator_new();

	gtk_box_pack_start(GTK_BOX(notebookvbox[2]), hbox[5], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[2]), hbox[6], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[2]), hbox[2], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[2]), hbox[4], FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(notebookvbox[2]), hbox[3], FALSE, FALSE, 3);


	gtk_widget_show_all(dialog);
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result)
	{
		case 1:
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiolevel[4])))
			{
				ptext = gtk_entry_get_text(GTK_ENTRY(entrytimeturn));
				if(is_integer(ptext))
				{
					sscanf(ptext, "%d", &timeoutturn);
					if (timeoutturn > 2000) timeoutturn = 2000;
					if (timeoutturn < 0) timeoutturn = 0;
					timeoutturn *= 1000;
					if (timeoutturn == 0) timeoutturn = 100;
				}
				ptext = gtk_entry_get_text(GTK_ENTRY(entrytimematch));
				if(is_integer(ptext))
				{
					sscanf(ptext, "%d", &timeoutmatch);
					if(timeoutmatch > 1000000) timeoutmatch = 1000000;
					if(timeoutmatch < 1) timeoutmatch = 1;
					timeoutmatch *= 1000;
					if(timeoutmatch < timeoutturn) timeoutmatch = timeoutturn;
				}
				ptext = gtk_entry_get_text(GTK_ENTRY(entrymaxdepth));
				if(is_integer(ptext))
				{
					sscanf(ptext, "%d", &maxdepth);
					if(maxdepth > boardsizeh*boardsizew) maxdepth = boardsizeh*boardsizew;
					if(maxdepth < 2) maxdepth = 2;
				}
				ptext = gtk_entry_get_text(GTK_ENTRY(entrymaxnode));
				if(is_integer(ptext))
				{
					sscanf(ptext, "%d", &maxnode);
					if(maxnode > 1000000) maxnode = 1000000;
					if(maxnode < 1) maxnode = 1;
					maxnode *= 1000;
				}
				set_level(4);
			}
			else
			{
				if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiolevel[0])))
				{
					set_level(0);
				}
				else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiolevel[1])))
				{
					set_level(1);
				}
				else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiolevel[2])))
				{
					set_level(2);
				}
				else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiolevel[3])))
				{
					set_level(3);
				}
				else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiolevel[5])))
				{
					set_level(5);
				}
				else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiolevel[6])))
				{
					set_level(6);
				}
				else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiolevel[7])))
				{
					set_level(7);
				}
				else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiolevel[8])))
				{
					set_level(8);
				}
			}

			if(gtk_range_get_value(GTK_RANGE(scalecaution)) < 0)
				set_cautionfactor((int)(gtk_range_get_value(GTK_RANGE(scalecaution))-1e-8));
			else
				set_cautionfactor((int)(gtk_range_get_value(GTK_RANGE(scalecaution))+1e-8));

			set_threadnum((int)(gtk_range_get_value(GTK_RANGE(scalethreads))+1e-8));

			set_threadsplitdepth((int)(gtk_range_get_value(GTK_RANGE(scalesplitdepth))+1e-8));

			set_hashsize((int)(gtk_range_get_value(GTK_RANGE(scalehash))+1e-8));

			set_pondering(gtk_toggle_button_get_active(hbox[5]) == TRUE ? 1 : 0);

			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiocheckmate[0])))
				set_checkmate(0);
			else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiocheckmate[1])))
				set_checkmate(1);
			else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiocheckmate[2])))
				set_checkmate(2);
			break;
		case 2:
			break;
	}
	gtk_widget_destroy(dialog);
}

void show_dialog_load(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	GtkFileFilter* filter;
	FILE *in;
	dialog = gtk_file_chooser_dialog_new("Load", data, GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Yixin saved positions");
	gtk_file_filter_add_pattern(filter, "*.[Ss][Aa][Vv]");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "POS");
	gtk_file_filter_add_pattern(filter, "*.[Pp][Oo][Ss]");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	gtk_window_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filenameutf, *filename;
		int nl;
		filenameutf = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		filename = g_locale_from_utf8(filenameutf, -1, NULL, NULL, NULL);
		printf_log("%s\n", filename);
		nl = strlen(filename);
		if((filename[nl-3] == 'P' || filename[nl-3] == 'p') &&
		   (filename[nl-2] == 'O' || filename[nl-2] == 'o') &&
		   (filename[nl-1] == 'S' || filename[nl-1] == 's'))
		{
			if((in = fopen(filename, "rb")) != NULL)
			{
				int i;
				unsigned char num;
				new_game(NULL, NULL);
				fread(&num, 1, 1, in);
				for(i=0; i<num; i++)
				{
					unsigned char xy;
					int x, y;
					fread(&xy, 1, 1, in);
					x = xy % 15;
					y = xy / 15;
					make_move(x, y);
					//printf_log("[%d/%d] %d %d (%d)\n", i, (int)num, x, y, (int)xy);
				}
				fclose(in);
				show_forbid();
			}
		}
		else if ((filename[nl-3] == 'S' || filename[nl-3] == 's') &&
		   (filename[nl-2] == 'A' || filename[nl-2] == 'a') &&
		   (filename[nl-1] == 'V' || filename[nl-1] == 'v'))
		{
			if((in = fopen(filename, "r")) != NULL)
			{
				int i, num;
				new_game(NULL, NULL);
				fscanf(in, "%*d"); //TODO: use boardsizeh?
				fscanf(in, "%*d"); //TODO: use boardsizew or inforule?
				/*
				fscanf(in, "%d", &inforule);
				switch(inforule)
				{
					case 0: gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemrule1), TRUE); break;
					case 1: gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemrule2), TRUE); break;
					case 2: gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemrule3), TRUE); break;
					case 3: gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemrule4), TRUE); break;
				}
				*/
				change_rule(NULL, (gpointer)inforule); //warning!
			
				fscanf(in, "%d", &num);
				for(i=0; i<num; i++)
				{
					int x, y;
					fscanf(in, "%d %d", &x, &y);
					make_move(x, y);
				}
				fclose(in);
				show_forbid();
			}
		}
		g_free(filenameutf);
		g_free(filename);
	}
	gtk_widget_destroy (dialog);
}
void show_dialog_save(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	GtkFileFilter* filter;
	FILE *out;
	dialog = gtk_file_chooser_dialog_new("Save", data, GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Yixin saved positions");
	gtk_file_filter_add_pattern(filter, "*.[Ss][Aa][Vv]");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	gtk_window_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filenameutf, *filename;
		char _filename[256];
		int len;
		filenameutf = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		filename = g_locale_from_utf8(filenameutf, -1, NULL, NULL, NULL);
		len = strlen(filename);
		if(len>=4 && (filename[len-1]=='V' || filename[len-1]=='v') &&
			(filename[len-2]=='A' || filename[len-2]=='a') &&
			(filename[len-3]=='S' || filename[len-3]=='s') &&
			(filename[len-4]=='.'))
		{
			sprintf(_filename, "%s", filename);
		}
		else
		{
			sprintf(_filename, "%s.sav", filename);
		}
		printf_log("%s\n", _filename);
		if((out = fopen(_filename, "w")) != NULL)
		{
			int i;
			fprintf(out, "%d\n", boardsizeh);
			fprintf(out, "%d\n", boardsizew);
			//fprintf(out, "%d\n", inforule);
			fprintf(out, "%d\n", piecenum);
			for(i=0; i<piecenum; i++)
			{
				fprintf(out, "%d %d\n", movepath[i]/boardsizew, movepath[i]%boardsizew);
			}
			fclose(out);
		}
		g_free(filenameutf);
		g_free(filename);
	}
	gtk_widget_destroy (dialog);
}

void show_dialog_size(GtkWidget *widget, gpointer data)
{
#ifdef G_OS_WIN32
	gchar *argv[] = {"Yixin.exe", NULL};
#else
	gchar *argv[] = {"./Yixin", NULL};
#endif
	gchar text[80];
	const gchar *ptext[2];
	GtkWidget *dialog;
	GtkWidget *table;
	GtkWidget *label[2];
	GtkWidget *entry[2];
	gint result;

	show_dialog_settings_custom_entry(NULL, 0);

	dialog = gtk_dialog_new_with_buttons("Settings", GTK_WINDOW(windowmain), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, "OK", 1, "Cancel", 2, NULL);
	gtk_window_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

	table = gtk_table_new(2, 2, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table), 0); /* set the row distance between elements to be 0 */
	gtk_table_set_col_spacings(GTK_TABLE(table), 0); /* set the column distance between elements to be 0 */
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), table, FALSE, FALSE, 3);
	
	label[0] = gtk_label_new(language == 0 ? "Board Height (10 ~ 24):" : _T(clanguage[42]));
	label[1] = gtk_label_new(language == 0 ? "Board Width (10 ~ 24):" : _T(clanguage[43]));
	
	entry[0] = gtk_entry_new();
	sprintf(text, "%d", boardsizeh);
	gtk_entry_set_text(GTK_ENTRY(entry[0]), text);

	entry[1] = gtk_entry_new();
	sprintf(text, "%d", boardsizew);
	gtk_entry_set_text(GTK_ENTRY(entry[1]), text);

	gtk_table_attach_defaults(GTK_TABLE(table), label[0], 0, 1, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(table), entry[0], 1, 2, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(table), label[1], 0, 1, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(table), entry[1], 1, 2, 1, 2);

	gtk_widget_show_all(dialog);
	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result)
	{
		case 1:
			ptext[0] = gtk_entry_get_text(GTK_ENTRY(entry[0]));
			ptext[1] = gtk_entry_get_text(GTK_ENTRY(entry[1]));
			if (is_integer(ptext[0]) && is_integer(ptext[1]))
			{
				int s1, s2;
				sscanf(ptext[0], "%d", &s1);
				sscanf(ptext[1], "%d", &s2);
				if (s1 <= MAX_SIZE && s1 >= 10 && s2 <= MAX_SIZE && s2 >= 10)
				{
					rboardsizeh = s1;
					rboardsizew = s2;
					if(boardsizeh != s1 || boardsizew != s2)
					{
						save_setting();
						g_spawn_async(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, NULL, NULL);
						yixin_quit();
					}
				}
			}
			break;
		case 2:
			break;
	}
	gtk_widget_destroy(dialog);
}

void show_dialog_custom_toolbar(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	GSList *ids;
	GtkListStore *store;
	GtkTreeModel *model;
	GtkWidget *combo;
	GtkWidget *cellview;
	GtkCellRenderer *renderer;
	GtkWidget *entry;
	GtkWidget *label[3];
	GtkWidget *table;
	GtkTextBuffer *buffercommand;
	GtkWidget *scrolledcommand;
	GtkWidget *textcommand;
	gchar text[80];
	GSList *p;
	gint result;
	const gchar *ptext;
	GtkTextIter start, end;

	int cnt;
	int i;
	int pi;

	dialog = gtk_dialog_new_with_buttons("Custom Toolbar", windowmain, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, "OK", 1, "Cancel", 2, NULL);
	gtk_window_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);	

	store = gtk_list_store_new(2, GDK_TYPE_PIXBUF, G_TYPE_STRING);
	cellview = gtk_cell_view_new();

	cnt = pi = 0;
	ids = gtk_stock_list_ids();
	ids = g_slist_sort(ids, (GCompareFunc)strcmp);
	p = ids;
	while (p != NULL)
	{
		gchar *id;
		GtkIconSet *icon;

		id = p->data;
		icon = gtk_icon_factory_lookup_default(id);
		if (icon)
		{
			gint n_sizes = 0;
			GtkIconSize *sizes = NULL;
			GtkIconSize size;
			int ok[3] = { 0 };

			gtk_icon_set_get_sizes(icon, &sizes, &n_sizes);

			size = sizes[0];
			i = 0;
			while (i < n_sizes)
			{
				if (sizes[i] == GTK_ICON_SIZE_SMALL_TOOLBAR) ok[0] = 1;
				if (sizes[i] == GTK_ICON_SIZE_LARGE_TOOLBAR) ok[1] = 1;
				if (sizes[i] == GTK_ICON_SIZE_BUTTON) ok[2] = 1;
				i++;
			}
			if (ok[0] && ok[1] && ok[2])
			{
				GdkPixbuf *pixbuf;
				char l[10];
				GtkTreeIter iter;
				cnt++;

				pixbuf = gtk_widget_render_icon(cellview, id,
					GTK_ICON_SIZE_BUTTON, NULL);

				sprintf(l, "Icon %d", cnt);
				gtk_list_store_append(store, &iter);
				gtk_list_store_set(store, &iter, 0, pixbuf, 1, l, -1);
				g_object_unref(pixbuf);

				if (strcmp(id, toolbaricon[(int)data]) == 0)
				{
					pi = cnt - 1;
				}
			}
		}

		p = g_slist_next(p);
	}

	gtk_widget_destroy(cellview);

	model = GTK_TREE_MODEL(store);
	combo = gtk_combo_box_new_with_model(model);
	g_object_unref(model);

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer, "pixbuf", 0, NULL);

	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer, "text", 1, NULL);

	gtk_combo_box_set_active(GTK_COMBO_BOX(combo), pi);

	entry = gtk_entry_new();
	sprintf(text, "%d", toolbarlng[(int)data]);
	gtk_entry_set_text(GTK_ENTRY(entry), text);

	label[0] = gtk_label_new(language == 0 ? "Icon:" : _T(clanguage[89]));
	label[1] = gtk_label_new(language == 0 ? "Text:" : _T(clanguage[90]));
	label[2] = gtk_label_new(language == 0 ? "Command:" : _T(clanguage[91]));

	textcommand = gtk_text_view_new();
	buffercommand = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textcommand));
	scrolledcommand = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolledcommand), textcommand);
	gtk_widget_set_size_request(scrolledcommand, 200, 200);

	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffercommand), &start, &end);
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffercommand), &end, toolbarcommand[(int)data], strlen(toolbarcommand[(int)data]));

	table = gtk_table_new(4, 2, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table), 0); /* set the row distance between elements to be 0 */
	gtk_table_set_col_spacings(GTK_TABLE(table), 0); /* set the column distance between elements to be 0 */
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), table, FALSE, FALSE, 3);

	gtk_table_attach_defaults(GTK_TABLE(table), label[0], 0, 1, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(table), combo, 1, 2, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(table), label[1], 0, 1, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(table), entry, 1, 2, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(table), label[2], 0, 2, 2, 3);
	gtk_table_attach_defaults(GTK_TABLE(table), scrolledcommand, 0, 2, 3, 4);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), table, FALSE, FALSE, 3);
	gtk_widget_show_all(dialog);
	result = gtk_dialog_run(GTK_DIALOG(dialog));

	switch (result)
	{
	case 1:
	{
		gchar *command;
#ifdef G_OS_WIN32
		gchar *argv[] = { "Yixin.exe", NULL };
#else
		gchar *argv[] = { "./Yixin", NULL };
#endif

		ptext = gtk_entry_get_text(GTK_ENTRY(entry));
		if (is_integer(ptext))
		{
			sscanf(ptext, "%d", &toolbarlng[(int)data]);
		}
		pi = gtk_combo_box_get_active(combo);
		cnt = 0;
		p = ids;
		while (p != NULL)
		{
			gchar *id;
			GtkIconSet *icon;

			id = p->data;
			icon = gtk_icon_factory_lookup_default(id);
			if (icon)
			{
				gint n_sizes = 0;
				GtkIconSize *sizes = NULL;
				GtkIconSize size;
				int ok[3] = { 0 };

				gtk_icon_set_get_sizes(icon, &sizes, &n_sizes);

				size = sizes[0];
				i = 0;
				while (i < n_sizes)
				{
					if (sizes[i] == GTK_ICON_SIZE_SMALL_TOOLBAR) ok[0] = 1;
					if (sizes[i] == GTK_ICON_SIZE_LARGE_TOOLBAR) ok[1] = 1;
					if (sizes[i] == GTK_ICON_SIZE_BUTTON) ok[2] = 1;
					i++;
				}
				if (ok[0] && ok[1] && ok[2])
				{
					GdkPixbuf *pixbuf;
					char l[10];
					GtkTreeIter iter;
					cnt++;

					pixbuf = gtk_widget_render_icon(cellview, id,
						GTK_ICON_SIZE_BUTTON, NULL);

					sprintf(l, "Icon %d", cnt);
					gtk_list_store_append(store, &iter);
					gtk_list_store_set(store, &iter, 0, pixbuf, 1, l, -1);
					g_object_unref(pixbuf);

					if (cnt - 1 == pi)
					{
						toolbaricon[(int)data] = strdup(id);
						break;
					}
				}
			}

			p = g_slist_next(p);
		}
		gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffercommand), &start, &end);
		command = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffercommand), &start, &end, FALSE);
		strcpy(toolbarcommand[(int)data], command);

		save_setting();
		g_spawn_async(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, NULL, NULL);
		yixin_quit();
	}
	case 2:
		break;
	}

	g_slist_foreach(ids, (GFunc)g_free, NULL);
	g_slist_free(ids);

	gtk_widget_destroy(dialog);
}

void show_dialog_custom_hotkey(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	GtkWidget *label[2];
	GtkWidget *table;
	GtkWidget *combo;
	GtkTextBuffer *buffercommand;
	GtkWidget *scrolledcommand;
	GtkWidget *textcommand;
	gchar text[80];
	gint result;
	const gchar *ptext;
	GtkTextIter start, end;
	
	int i;

	dialog = gtk_dialog_new_with_buttons("Custom Hotkey", windowmain, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, "OK", 1, "Cancel", 2, NULL);
	gtk_window_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

	combo = gtk_combo_box_new_text();
	for (i = 0; hotkeynamelis[i] != NULL; i++)
	{
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo), hotkeynamelis[i]);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo), hotkeykey[(int)data]);

	label[0] = gtk_label_new(language == 0 ? "Hotkey:" : _T(clanguage[93]));
	label[1] = gtk_label_new(language == 0 ? "Command:" : _T(clanguage[91]));

	textcommand = gtk_text_view_new();
	buffercommand = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textcommand));
	scrolledcommand = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolledcommand), textcommand);
	gtk_widget_set_size_request(scrolledcommand, 200, 200);

	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffercommand), &start, &end);
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffercommand), &end, hotkeycommand[(int)data], strlen(hotkeycommand[(int)data]));

	table = gtk_table_new(4, 2, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table), 0); /* set the row distance between elements to be 0 */
	gtk_table_set_col_spacings(GTK_TABLE(table), 0); /* set the column distance between elements to be 0 */
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), table, FALSE, FALSE, 3);

	gtk_table_attach_defaults(GTK_TABLE(table), label[0], 0, 1, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(table), combo, 1, 2, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(table), label[1], 0, 1, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(table), scrolledcommand, 0, 2, 2, 3);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), table, FALSE, FALSE, 3);
	gtk_widget_show_all(dialog);
	result = gtk_dialog_run(GTK_DIALOG(dialog));

	switch (result)
	{
	case 1:
	{
		gchar *command;

		gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffercommand), &start, &end);
		command = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffercommand), &start, &end, FALSE);
		strcpy(hotkeycommand[(int)data], command);
		hotkeykey[(int)data] = gtk_combo_box_get_active(combo);
	}
	case 2:
		break;
	}

	gtk_widget_destroy(dialog);
}

void show_dialog_about(GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;
	GdkPixbuf *pixbuf;
	GtkWidget *icon;
	GtkWidget *name;
	GtkWidget *version;
	GtkWidget *author;
	GtkWidget *www;

	show_thanklist();

	dialog = gtk_dialog_new_with_buttons("About", data, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, "OK", 1, NULL);
	gtk_window_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	
	pixbuf = gdk_pixbuf_new_from_file("icon.ico", NULL);

	icon = gtk_image_new_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);
	pixbuf = NULL;
	name = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(name), "<big><b>Yixin Board</b></big>");
	version = gtk_label_new("Version "VERSION);
	author = gtk_label_new("(C)2009-2016 Kai Sun");
	www = gtk_label_new("www.aiexp.info");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), icon, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), name, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), version, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), author, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), www, FALSE, FALSE, 10);

	gtk_widget_show_all(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

void new_game(GtkWidget *widget, gpointer data)
{
	//GtkTextIter start, end;

	piecenum = 0;
	isgameover = 0;
	timeused = 0;
	memset(board, 0, sizeof(board));
	memset(forbid, 0, sizeof(forbid));
	memset(bestline, 0, sizeof(bestline));
	memset(boardbest, 0, sizeof(boardbest));
	memset(boardlose, 0, sizeof(boardlose));
	memset(boardpos, 0, sizeof(boardpos));
	refresh_board();
	if(isthinking) isneedomit ++;
	isthinking = 0;
	isneedrestart = 1;

	if(widget != NULL) refreshboardflag = 0;
	//gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffertextlog), &start, &end);
	//gtk_text_buffer_delete(buffertextlog, &start, &end);
}
void set_rule()
{
	char command[80];
	//printf_log("INFO rule %d\n", inforule);
	sprintf(command, "INFO rule %d\n", inforule);
	send_command(command);
	isneedrestart = 1;
	show_forbid();
}
void change_rule(GtkWidget *widget, gpointer data)
{
	inforule = (int)data;
	if(inforule == 3)
	{
		inforule = 2;
		specialrule = 1;
	}
	else if(inforule == 4)
	{
		inforule = 0;
		specialrule = 2;
	}
	else
	{
		specialrule = 0;
	}
	set_rule();
}

void change_side(GtkWidget *widget, gpointer data)
{
	computerside ^= (int)data;
	isneedrestart = 1;
}
void change_side_menu(int flag, GtkWidget *w)
{
	static GtkWidget *rec[2];
	switch(flag)
	{
		case 3: rec[0] = w; break;
		case 4: rec[1] = w; break;
		case -1: gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(rec[0]), FALSE); break;
		case -2: gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(rec[1]), FALSE); break;
		case 1: gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(rec[0]), TRUE); break;
		case 2: gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(rec[1]), TRUE); break;
	}
}

void use_openbook(GtkWidget *widget, gpointer data)
{
	useopenbook ^= 1;
}
void view_analysis(GtkWidget *widget, gpointer data)
{
	showanalysis ^= 1;
	refresh_board();
}
void view_numeration(GtkWidget *widget, gpointer data)
{
	shownumber ^= 1;
	refresh_board();
}
void view_log(GtkWidget *widget, gpointer data)
{
	showlog ^= 1;
	if(showlog)
	{
		gtk_widget_show(scrolledtextlog);
		gtk_widget_show(scrolledtextcommand);
		gtk_widget_show(toolbar);
	}
	else
	{
		gtk_widget_hide(scrolledtextlog);
		gtk_widget_hide(scrolledtextcommand);
		gtk_widget_hide(toolbar);
	}
}
void change_language(GtkWidget *widget, gpointer data)
{
#ifdef G_OS_WIN32
	gchar *argv[] = {"Yixin.exe", NULL};
#else
	gchar *argv[] = {"./Yixin", NULL};
#endif
	if(language == (int)data) return;
	rlanguage = (int)data;
	save_setting();
	g_spawn_async(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, NULL, NULL);
	yixin_quit();
}
void change_piece(GtkWidget *widget, gpointer data)
{
	int i;
	int p = 0;
	if(widget != NULL) refreshboardflag = 0;
	switch((int)data)
	{
	case 0: p = 0; break;
	case 1: p = piecenum - 1; break;
	case 2: p = piecenum + 1; break;
	case 3: p = MAX_SIZE*MAX_SIZE; break;
	}
	while(p > 0 && movepath[p-1] == -1) p --;

	new_game(NULL, NULL);
	for(i=0; i<p; i++) make_move(movepath[i]/boardsizew, movepath[i]%boardsizew);
	show_forbid();

	stop_thinking(widget, data);
}
void stop_thinking(GtkWidget *widget, gpointer data)
{
	char command[80];
	sprintf(command, "yxstop\n");
	send_command(command);
}

void start_thinking(GtkWidget *widget, gpointer data)
{
	GdkEventButton event;
	GdkWindowEdge edge = GDK_WINDOW_EDGE_NORTH_WEST; // not useful
	if(isthinking) return;
	if(piecenum%2 == 1 && (computerside & 1))
		change_side_menu(-1, NULL);
	if(piecenum%2 == 0 && (computerside & 2))
		change_side_menu(-2, NULL);
	if(piecenum%2 == 0 && computerside == 0)
		change_side_menu(1, NULL);
	if(piecenum%2 == 1 && computerside == 0)
		change_side_menu(2, NULL);
	event.type = GDK_BUTTON_PRESS;
	event.button = 1;
	event.x = imageboard[0][0]->allocation.x;
	event.y = imageboard[0][0]->allocation.y;
	on_button_press_windowmain(widget, &event, edge);
	if(computerside == 1)
		change_side_menu(-1, NULL);
	if(computerside == 2)
		change_side_menu(-2, NULL);
}

void custom_function(char *command)
{
	int l = 0, r;
	while (command[l])
	{
		char t;
		if (command[l] == '\n' || command[l] == '\r')
		{
			l++;
			continue;
		}
		r = l + 1;
		while (command[r] && command[r] != '\n') r++;

		t = command[r + 1];
		command[r + 1] = 0;
		execute_command(command + l);
		command[r + 1] = t;
		l = r;
	}
}

void hotkey_function(GtkWidget *widget, gpointer data)
{
	custom_function(hotkeycommand[(int)data]);
}

void toolbar_function(GtkWidget *widget, gpointer data)
{
	custom_function(toolbarcommand[(int)data]);
}

void execute_command(gchar *command)
{
	int i;
	if (_strnicmp(command, "command on", 10) == 0)
	{
		commandmodel = 1;
	}
	else if (_strnicmp(command, "command off", 11) == 0)
	{
		commandmodel = 0;
	}
	else if (commandmodel == 1)
	{
		printf_log(command);
		send_command(command);
	}
	else if (_strnicmp(command, "help", 4) == 0)
	{
		if (language)
		{
			printf_log(clanguage[50]);
		}
		else
		{
			printf_log("Command Lists:");
		}
		printf_log("\n");
		printf_log(" help\n");
		printf_log(" clear\n");
		printf_log(" rotate [90,180,270]\n");
		printf_log(" flip [/,\\,-,|]\n");
		printf_log(" move [^,v,<,>]\n");
		printf_log(" getpos\n");
		printf_log(" putpos\n");
		printf_log("   %s: putpos f11h7g10h6i10h5j11h8h9h4\n", language ? clanguage[51] : "Example");
		printf_log(" block\n");
		printf_log("   %s: block h8\n", language ? clanguage[51] : "Example");
		printf_log(" block undo\n");
		printf_log("   %s: block undo h8\n", language ? clanguage[51] : "Example");
		printf_log(" block reset\n");
		printf_log(" block compare\n");
		printf_log("   %s: block compare h8i8j7\n", language ? clanguage[51] : "Example");
		printf_log(" block autoreset [on,off]\n");
		printf_log(" blockpath\n");
		printf_log("   %s: blockpath h8h7\n", language ? clanguage[51] : "Example");
		printf_log(" blockpath undo\n");
		printf_log("   %s: blockpath undo h8h7\n", language ? clanguage[51] : "Example");
		printf_log(" blockpath reset\n");
		printf_log(" blockpath except\n");
		printf_log("   %s: blockpath except h8i8j7\n", language ? clanguage[51] : "Example");
		printf_log(" blockpath autoreset [on,off]\n");
		printf_log(" hash clear\n");
		printf_log(" hash autoclear [on,off]\n");
		printf_log(" hash dump [filename]\n");
		printf_log(" hash load [filename]\n");
		printf_log(" bestline\n");
		printf_log(" balance<1,2>\n");
		printf_log("   %s: balance1\n", language ? clanguage[52] : "Example 1");
		printf_log("   %s: balance1 100\n", language ? clanguage[53] : "Example 2");
		printf_log("   %s: balance2\n", language ? clanguage[54] : "Example 3");
		printf_log("   %s: balance2 100\n", language ? clanguage[55] : "Example 4");
		printf_log(" nbest [2,3,...]\n");
		printf_log(" search from [depth]\n");
		printf_log(" toolbar edit [1,2,...]\n");
		printf_log(" toolbar add\n");
		printf_log(" toolbar remove\n");
		printf_log(" key edit [1,2,...]\n");
		printf_log(" key add\n");
		printf_log(" key remove\n");
		printf_log(" key list\n");
		printf_log(" thinking start\n");
		printf_log(" thinking stop\n");
		printf_log(" undo one\n");
		printf_log(" undo all\n");
		printf_log(" redo one\n");
		printf_log(" redo all\n");
		printf_log(" command [on,off]\n");
		printf_log(" hash usage\n");
		//printf_log(" sleep [second]\n");
		//printf_log("   %s: sleep 5\n", language ? clanguage[51] : "Example");
		printf_log("\n");
	}
	else if (_strnicmp(command, "clear", 5) == 0)
	{
		GtkTextIter _start, _end;
		gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffertextlog), &_start, &_end);
		gtk_text_buffer_delete(buffertextlog, &_start, &_end);
	}
	else if (_strnicmp(command, "rotate", 6) == 0)
	{
		int p = piecenum;
		int j, k = 1;
		if (boardsizew != boardsizeh)
		{
			printf_log(language == 0 ? "Sorry, board cannot be rotated when height<>width." : clanguage[56]);
			printf_log("\n");
		}
		else
		{
			if (strlen(command) >= 8)
			{
				if (command[7] == '9') k = 1;
				else if (command[7] == '1') k = 2;
				else if (command[7] == '2') k = 3;
			}
			refreshboardflag = 0;
			for (j = 0; j < k; j++)
			{
				for (i = 0; i < p; i++)
				{
					int _x, _y, x, y;
					_y = movepath[i] / boardsizew;
					_x = movepath[i] % boardsizew;
					y = _x;
					x = boardsizeh - 1 - _y;
					movepath[i] = y*boardsizew + x;
				}
			}
			new_game(NULL, NULL);
			for (i = 0; i < p; i++) make_move(movepath[i] / boardsizew, movepath[i] % boardsizew);
			show_forbid();
		}
	}
	else if (_strnicmp(command, "flip", 4) == 0)
	{
		int p = piecenum;
		int k = 0;
		if (strlen(command) >= 6)
		{
			if (command[5] == '-') k = 0;
			else if (command[5] == '|') k = 1;
			else if (command[5] == '/') k = 2;
			else if (command[5] == '\\') k = 3;
		}
		if ((k == 2 || k == 3) && (boardsizew != boardsizeh))
		{
			printf_log(language == 0 ? "Sorry, board cannot be flipped with / or \\ when height<>width." : clanguage[57]);
			printf_log("\n");
		}
		else
		{
			refreshboardflag = 0;
			for (i = 0; i < p; i++)
			{
				int _x, _y, x, y;
				_y = movepath[i] / boardsizew;
				_x = movepath[i] % boardsizew;
				switch (k)
				{
				case 0:
					y = boardsizeh - 1 - _y;
					x = _x;
					break;
				case 1:
					y = _y;
					x = boardsizew - 1 - _x;
					break;
				case 2:
					y = boardsizeh - 1 - _y;
					x = boardsizew - 1 - _x;
					break;
				case 3:
					y = _x;
					x = _y;
					break;
				}
				movepath[i] = y*boardsizew + x;
			}
			new_game(NULL, NULL);
			for (i = 0; i < p; i++) make_move(movepath[i] / boardsizew, movepath[i] % boardsizew);
			show_forbid();
		}
	}
	else if (_strnicmp(command, "move", 4) == 0)
	{
		int p = piecenum;
		int k = 0;
		int f = 1;
		if (strlen(command) >= 6)
		{
			if (command[5] == '^') k = 0;
			else if (command[5] == 'v') k = 1;
			else if (command[5] == '<') k = 2;
			else if (command[5] == '>') k = 3;
		}
		refreshboardflag = 0;
		for (i = 0; i<p; i++)
		{
			int _x, _y, x, y;
			_y = movepath[i] / boardsizew;
			_x = movepath[i] % boardsizew;
			switch (k)
			{
			case 0:
				y = _y - 1;
				x = _x;
				break;
			case 1:
				y = _y + 1;
				x = _x;
				break;
			case 2:
				y = _y;
				x = _x - 1;
				break;
			case 3:
				y = _y;
				x = _x + 1;
				break;
			}
			if (x < 0 || x > boardsizew - 1 || y < 0 || y > boardsizeh - 1) f = 0;
		}
		if (f)
		{
			for (i = 0; i < p; i++)
			{
				int _x, _y, x, y;
				_y = movepath[i] / boardsizew;
				_x = movepath[i] % boardsizew;
				switch (k)
				{
				case 0:
					y = _y - 1;
					x = _x;
					break;
				case 1:
					y = _y + 1;
					x = _x;
					break;
				case 2:
					y = _y;
					x = _x - 1;
					break;
				case 3:
					y = _y;
					x = _x + 1;
					break;
				}
				movepath[i] = y*boardsizew + x;
			}
		}
		new_game(NULL, NULL);
		for (i = 0; i < p; i++) make_move(movepath[i] / boardsizew, movepath[i] % boardsizew);
		show_forbid();
	}
	else if (_strnicmp(command, "putpos", 6) == 0)
	{
		new_game(NULL, NULL);
		i = 6;
		while (command[i] == '\t' || command[i] == ' ') i++;
		for (; command[i];)
		{
			int x, y;
			if (command[i] >= 'a' && command[i] <= 'z') command[i] = command[i] - 'a' + 'A';
			if (command[i] < 'A' || command[i] > 'Z') break;
			x = command[i] - 'A';
			i++;
			y = command[i] - '0';
			i++;
			if (command[i] >= '0' && command[i] <= '9')
			{
				y = y * 10 + command[i] - '0';
				i++;
			}
			y = y - 1;
			if (x < 0 || x >= boardsizew || y < 0 || y >= boardsizeh) break;
			make_move(boardsizeh - 1 - y, x);
		}
		show_forbid();
	}
	else if (_strnicmp(command, "getpos", 6) == 0)
	{
		for (i = 0; i < piecenum; i++)
		{
			printf_log("%c%d", movepath[i] % boardsizew + 'a', boardsizeh - 1 - movepath[i] / boardsizew + 1);
		}
		printf_log("\n");
	}
	else if (_strnicmp(command, "blockpath reset", 15) == 0)
	{
		send_command("yxblockpathreset\n");
	}
	else if (_strnicmp(command, "blockpath autoreset", 19) == 0)
	{
		if (strlen(command) >= 22)
		{
			if (command[21] == 'n' || command[21] == 'N')
			{
				blockpathautoreset = 1;
			}
			else
			{
				blockpathautoreset = 0;
			}
		}
	}
	else if (_strnicmp(command, "blockpath undo", 14) == 0)
	{
		gchar _command[80];
		int xl[MAX_SIZE*MAX_SIZE], yl[MAX_SIZE*MAX_SIZE];
		int len = 0;

		i = 14;
		while (command[i] == '\t' || command[i] == ' ') i++;
		for (; command[i];)
		{
			int x, y;
			if (command[i] >= 'a' && command[i] <= 'z') command[i] = command[i] - 'a' + 'A';
			if (command[i] < 'A' || command[i] > 'Z') break;
			x = command[i] - 'A';
			i++;
			y = command[i] - '0';
			i++;
			if (command[i] >= '0' && command[i] <= '9')
			{
				y = y * 10 + command[i] - '0';
				i++;
			}
			y = y - 1;
			if (x < 0 || x >= boardsizew || y < 0 || y >= boardsizeh) break;
			xl[len] = boardsizeh - 1 - y;
			yl[len] = x;
			len++;
		}
		if (len > 0)
		{
			send_command("yxblockpathundo\n");
			for (i = 0; i < piecenum; i++)
			{
				sprintf(_command, "%d,%d\n", movepath[i] / boardsizew, movepath[i] % boardsizew);
				send_command(_command);
			}
			for (i = 0; i < len; i++)
			{
				sprintf(_command, "%d,%d\n", xl[i], yl[i]);
				send_command(_command);
			}
			send_command("done\n");
		}
	}
	else if (_strnicmp(command, "blockpath except", 16) == 0)
	{
		gchar _command[80];
		int xl[MAX_SIZE*MAX_SIZE], yl[MAX_SIZE*MAX_SIZE];
		int len = 0;

		i = 16;
		while (command[i] == '\t' || command[i] == ' ') i++;
		for (; command[i];)
		{
			int x, y;
			if (command[i] >= 'a' && command[i] <= 'z') command[i] = command[i] - 'a' + 'A';
			if (command[i] < 'A' || command[i] > 'Z') break;
			x = command[i] - 'A';
			i++;
			y = command[i] - '0';
			i++;
			if (command[i] >= '0' && command[i] <= '9')
			{
				y = y * 10 + command[i] - '0';
				i++;
			}
			y = y - 1;
			if (x < 0 || x >= boardsizew || y < 0 || y >= boardsizeh) break;
			xl[len] = boardsizeh - 1 - y;
			yl[len] = x;
			len++;
		}
		if (len > 0)
		{
			int j, k;
			for (j = 0; j < MAX_SIZE; j++)
			{
				for (k = 0; k < MAX_SIZE; k++)
				{
					if (xl[len - 1] == j && yl[len - 1] == k) continue;
					send_command("yxblockpath\n");
					for (i = 0; i < piecenum; i++)
					{
						sprintf(_command, "%d,%d\n", movepath[i] / boardsizew, movepath[i] % boardsizew);
						send_command(_command);
					}
					for (i = 0; i < len - 1; i++)
					{
						sprintf(_command, "%d,%d\n", xl[i], yl[i]);
						send_command(_command);
					}
					sprintf(_command, "%d,%d\n", j, k);
					send_command(_command);
					send_command("done\n");
				}
			}
		}
	}
	else if (_strnicmp(command, "blockpath list", 14) == 0)
	{
		; //TODO
	}
	else if (_strnicmp(command, "blockpath", 9) == 0)
	{
		gchar _command[80];
		int xl[MAX_SIZE*MAX_SIZE], yl[MAX_SIZE*MAX_SIZE];
		int len = 0;

		i = 9;
		while (command[i] == '\t' || command[i] == ' ') i++;
		for (; command[i];)
		{
			int x, y;
			if (command[i] >= 'a' && command[i] <= 'z') command[i] = command[i] - 'a' + 'A';
			if (command[i] < 'A' || command[i] > 'Z') break;
			x = command[i] - 'A';
			i++;
			y = command[i] - '0';
			i++;
			if (command[i] >= '0' && command[i] <= '9')
			{
				y = y * 10 + command[i] - '0';
				i++;
			}
			y = y - 1;
			if (x < 0 || x >= boardsizew || y < 0 || y >= boardsizeh) break;
			xl[len] = boardsizeh - 1 - y;
			yl[len] = x;
			len++;
		}
		if (len > 0)
		{
			send_command("yxblockpath\n");
			for (i = 0; i < piecenum; i++)
			{
				sprintf(_command, "%d,%d\n", movepath[i] / boardsizew, movepath[i] % boardsizew);
				send_command(_command);
			}
			for (i = 0; i < len; i++)
			{
				sprintf(_command, "%d,%d\n", xl[i], yl[i]);
				send_command(_command);
			}
			send_command("done\n");
		}
	}
	else if (_strnicmp(command, "block reset", 11) == 0)
	{
		send_command("yxblockreset\n");
		memset(boardblock, 0, sizeof(boardblock));
		refresh_board();
	}
	else if (_strnicmp(command, "block autoreset", 15) == 0)
	{
		if (strlen(command) >= 18)
		{
			if (command[17] == 'n' || command[17] == 'N')
			{
				blockautoreset = 1;
			}
			else
			{
				blockautoreset = 0;
			}
		}
	}
	else if (_strnicmp(command, "block undo", 10) == 0)
	{
		gchar _command[80];
		do
		{
			int x, y;
			if (command[11] >= 'a' && command[11] <= 'z') command[11] = command[11] - 'a' + 'A';
			if (command[11] < 'A' || command[11] > 'Z') break;
			x = command[11] - 'A';
			y = command[12] - '0';
			if (command[13] >= '0' && command[13] <= '9')
			{
				y = y * 10 + command[13] - '0';
			}
			y--;
			if (x<0 || x >= boardsizew || y<0 || y >= boardsizeh) break;
			send_command("yxblockundo\n");
			sprintf(_command, "%d,%d\n", boardsizeh - 1 - y, x);
			send_command(_command);
			send_command("done\n");

			boardblock[boardsizeh - 1 - y][x] = 0;
			refresh_board();
		} while (0);
	}
	else if (_strnicmp(command, "block compare", 13) == 0)
	{
		gchar _command[80];
		int j;
		send_command("yxblockreset\n");
		memset(boardblock, 0, sizeof(boardblock));
		for (i = 0; i < boardsizeh; i++)
		{
			for (j = 0; j < boardsizew; j++)
			{
				if (board[i][j] == 0)
				{
					boardblock[i][j] = 1;
				}
			}
		}
		i = 13;
		while (command[i] == '\t' || command[i] == ' ') i++;
		for (; command[i];)
		{
			int x, y;
			if (command[i] >= 'a' && command[i] <= 'z') command[i] = command[i] - 'a' + 'A';
			if (command[i] < 'A' || command[i] > 'Z') break;
			x = command[i] - 'A';
			i++;
			y = command[i] - '0';
			i++;
			if (command[i] >= '0' && command[i] <= '9')
			{
				y = y * 10 + command[i] - '0';
				i++;
			}
			y = y - 1;
			if (x < 0 || x >= boardsizew || y < 0 || y >= boardsizeh) break;
			boardblock[boardsizeh - 1 - y][x] = 0;
		}
		for (i = 0; i < boardsizeh; i++)
		{
			for (j = 0; j < boardsizew; j++)
			{
				if (boardblock[i][j] == 1)
				{
					send_command("yxblock\n");
					sprintf(_command, "%d,%d\n", i, j);
					send_command(_command);
					send_command("done\n");
				}
			}
		}
		refresh_board();
	}
	else if (_strnicmp(command, "block", 5) == 0)
	{
		gchar _command[80];
		i = 0;
		do
		{
			int x, y;
			if (command[6 + i] >= 'a' && command[6 + i] <= 'z') command[6 + i] = command[6 + i] - 'a' + 'A';
			if (command[6 + i] < 'A' || command[6 + i] > 'Z') break;
			x = command[6 + i] - 'A';
			y = command[7 + i] - '0';
			if (command[8 + i] >= '0' && command[8 + i] <= '9')
			{
				y = y * 10 + command[8 + i] - '0';
				i++;
			}
			i += 2;

			y--;
			if (x<0 || x >= boardsizew || y<0 || y >= boardsizeh) break;
			send_command("yxblock\n");
			sprintf(_command, "%d,%d\n", boardsizeh - 1 - y, x);
			send_command(_command);
			send_command("done\n");

			boardblock[boardsizeh - 1 - y][x] = 1;
		} while (isalpha(command[6 + i]));
		refresh_board();
	}
	else if (_strnicmp(command, "hash autoclear", 14) == 0)
	{
		if (strlen(command) >= 17)
		{
			if (command[16] == 'n' || command[16] == 'N')
			{
				hashautoclear = 1;
			}
			else
			{
				hashautoclear = 0;
			}
		}
	}
	else if (_strnicmp(command, "hash clear", 10) == 0)
	{
		send_command("yxhashclear\n");
	}
	else if (_strnicmp(command, "hash dump", 9) == 0)
	{
		gchar _command[80];
		sprintf(_command, "%s", command + 9 + 1);
		i = strlen(_command);
		while (i > 0)
		{
			if (_command[i - 1] == '\n' || _command[i - 1] == '\r')
			{
				_command[i - 1] = 0;
				i--;
			}
			else
				break;
		}
		_command[i] = '\n';
		_command[i + 1] = 0;
		if (i > 0)
		{
			send_command("yxhashdump\n");
			send_command(_command);
		}
	}
	else if (_strnicmp(command, "hash load", 9) == 0)
	{
		gchar _command[80];
		sprintf(_command, "%s", command + 9 + 1);
		i = strlen(_command);
		while (i > 0)
		{
			if (_command[i - 1] == '\n' || _command[i - 1] == '\r')
			{
				_command[i - 1] = 0;
				i--;
			}
			else
				break;
		}
		_command[i] = '\n';
		_command[i + 1] = 0;
		if (i > 0)
		{
			send_command("yxhashload\n");
			send_command(_command);
		}
	}
	else if (_strnicmp(command, "hash usage", 10) == 0)
	{
		send_command("yxshowhashusage\n");
	}
	else if (_strnicmp(command, "search from", 11) == 0)
	{
		gchar _command[80];
		int depth = 1;
		sscanf(command + 11 + 1, "%d", &depth);
		sprintf(_command, "info start_depth %d\n", depth);
		send_command(_command);
	}
	else if (_strnicmp(command, "key list", 8) == 0)
	{
		for (i = 0; i < hotkeynum; i++)
		{
			printf_log("%d. %s\n", i + 1, hotkeynamelis[hotkeykey[i]]);
		}
		printf_log("\n");
	}
	else if (_strnicmp(command, "key edit", 8) == 0)
	{
		int n;
		if (sscanf(command + 8 + 1, "%d", &n) != EOF)
		{
			show_dialog_custom_hotkey(NULL, (gpointer)(n - 1));
		}
	}
	else if (_strnicmp(command, "key add", 7) == 0)
	{
		int n;
		if (hotkeynum < MAX_HOTKEY_ITEM)
		{
			hotkeynum++;
			strcpy(hotkeycommand[hotkeynum - 1], "\n");
			hotkeykey[hotkeynum - 1] = 0;
			show_dialog_custom_hotkey(NULL, (gpointer)(hotkeynum - 1));
		}
	}
	else if (_strnicmp(command, "key remove", 10) == 0)
	{
		if (hotkeynum > 0)
		{
			hotkeynum--;
		}
	}
	else if (_strnicmp(command, "toolbar edit", 12) == 0)
	{
		int n;
		if (sscanf(command + 12 + 1, "%d", &n) != EOF)
		{
			show_dialog_custom_toolbar(NULL, (gpointer)(n - 1));
		}
	}
	else if (_strnicmp(command, "toolbar add", 11) == 0)
	{
		int n;
		if (toolbarnum < MAX_TOOLBAR_ITEM)
		{
			toolbarnum++;
			toolbarlng[toolbarnum - 1] = 92;
			toolbaricon[toolbarnum - 1] = strdup("gtk-about");
			strcpy(toolbarcommand[toolbarnum - 1], "\n");
			show_dialog_custom_toolbar(NULL, (gpointer)(toolbarnum-1));
		}
	}
	else if (_strnicmp(command, "toolbar remove", 14) == 0)
	{
		int n;
#ifdef G_OS_WIN32
		gchar *argv[] = { "Yixin.exe", NULL };
#else
		gchar *argv[] = { "./Yixin", NULL };
#endif
		if (toolbarnum > 0)
		{
			toolbarnum--;
			save_setting();
			g_spawn_async(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, NULL, NULL);
			yixin_quit();
		}
	}
	else if (_strnicmp(command, "thinking start", 14) == 0)
	{
		start_thinking(windowmain, NULL);
	}
	else if (_strnicmp(command, "thinking stop", 13) == 0)
	{
		stop_thinking(windowmain, NULL);
	}
	else if (_strnicmp(command, "undo all", 8) == 0)
	{
		change_piece(windowmain, (gpointer)0);
	}
	else if (_strnicmp(command, "undo one", 8) == 0)
	{
		change_piece(windowmain, (gpointer)1);
	}
	else if (_strnicmp(command, "redo one", 8) == 0)
	{
		change_piece(windowmain, (gpointer)2);
	}
	else if (_strnicmp(command, "redo all", 8) == 0)
	{
		change_piece(windowmain, (gpointer)3);
	}
	/*
	else if (_strnicmp(command, "sleep", 5) == 0)
	{
		int n;
		sscanf(command + 5 + 1, "%d", &n);
		//TODO
	}
	*/
	else if (_strnicmp(command, "bestline", 8) == 0)
	{
		printf_log("BESTLINE: %s ", bestline);
		printf_log("VAL: %d\n", bestval);
	}
	else if (_strnicmp(command, "balance", 7) == 0 && (command[7] == '1' || command[7] == '2'))
	{
		gchar _command[80];
		int s;
		int t;
		t = command[7] - '0';
		if (sscanf(command + 8, "%d", &s) != 1) s = 0;
		sprintf(_command, "start %d %d\n", boardsizew, boardsizeh);
		send_command(_command);
		sprintf(_command, "yxboard\n");
		send_command(_command);
		for (i = 0; i<piecenum; i++)
		{
			sprintf(_command, "%d,%d,%d\n", movepath[i] / boardsizew,
				movepath[i] % boardsizew, piecenum % 2 == i % 2 ? 1 : 2);
			send_command(_command);
		}
		sprintf(_command, "done\n");
		send_command(_command);
		sprintf(_command, "yxbalance%s %d\n", t == 1 ? "one" : "two", s);
		send_command(_command);
	}
	else if (_strnicmp(command, "nbest", 5) == 0)
	{
		gchar _command[80];
		int s;
		if (sscanf(command + 5, "%d", &s) != 1) s = 2;
		sprintf(_command, "start %d %d\n", boardsizew, boardsizeh);
		send_command(_command);
		sprintf(_command, "yxboard\n");
		send_command(_command);
		for (i = 0; i<piecenum; i++)
		{
			sprintf(_command, "%d,%d,%d\n", movepath[i] / boardsizew,
				movepath[i] % boardsizew, piecenum % 2 == i % 2 ? 1 : 2);
			send_command(_command);
		}
		sprintf(_command, "done\n");
		send_command(_command);
		sprintf(_command, "yxnbest %d\n", s);
		send_command(_command);
	}
	else if (_strnicmp(command, "makebook", 8) == 0)
	{
		; //TODO
	}
	else
	{
		if (language)
		{
			printf_log(clanguage[58]);
		}
		else
		{
			printf_log("To get help, type help and press Enter");
		}
		printf_log("\n");
	}
}

gboolean key_command(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	GtkTextIter start, end;
	gchar *command;

	if(event->keyval == 0xff0d) // warning: 0xff0d == GDK_KEY_Return
	{
		gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffertextcommand), &start, &end);
		command = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffertextcommand), &start, &end, FALSE);
		
		execute_command(command);

		gtk_text_buffer_delete(buffertextcommand, &start, &end);
		g_free(command);
	}
	return FALSE;
}

void save_setting()
{
	FILE *out;
	int i;
	char s[80];

	if((out = fopen("settings.txt", "w")) != NULL)
	{
		fprintf(out, "%d %d\t;board size (10 ~ %d)\n", rboardsizeh, rboardsizew, MAX_SIZE);
		fprintf(out, "%d\t;language (0: English, 1,2,...: custom)\n", rlanguage);
		fprintf(out, "%d\t;rule (0: freestyle, 1: standard, 2: free renju, 3: swap after 1st move)\n", specialrule==2?3:(specialrule==1?4:inforule));
		fprintf(out, "%d\t;openbook (0: not use, 1: use)\n", useopenbook);
		fprintf(out, "%d\t;computer play black (0: no, 1: yes)\n", computerside&1);
		fprintf(out, "%d\t;computer play white (0: no, 1: yes)\n", computerside>>1);
		fprintf(out, "%d\t;level (0: 4dan, 1: 3dan, 2: 2dan, 3: 1dan, 5: 6dan, 6: 9dan, 7: meijin, 8: unlimited time 4: custom level)\n", levelchoice);
		fprintf(out, "%d\t;time limit (turn)\n", timeoutturn/1000);
		fprintf(out, "%d\t;time limit (match)\n", timeoutmatch/1000);
		fprintf(out, "%d\t;max depth\n", maxdepth);
		fprintf(out, "%d\t;max node\n", maxnode);
		fprintf(out, "%d\t;style (rash 0 ~ %d cautious)\n", cautionfactor, CAUTION_NUM);
		fprintf(out, "%d\t;toolbar style (0: only icon, 1: both icon and words)\n", showtoolbarboth);
		fprintf(out, "%d\t;show log (0: no, 1: yes)\n", showlog);
		fprintf(out, "%d\t;show number (0: no, 1: yes)\n", shownumber);
		fprintf(out, "%d\t;show analysis (0: no, 1: yes)\n", showanalysis);
		fprintf(out, "%d\t;show warning (0: no, 1: yes)\n", showwarning);
		fprintf(out, "%d\t;block autoreset (0: no, 1: yes)\n", blockautoreset);
		fprintf(out, "%d\t;number of threads\n", threadnum);
		fprintf(out, "%d\t;hash size\n", hashsize);
		fprintf(out, "%d\t;split depth\n", threadsplitdepth);
		fprintf(out, "%d\t;blockpath autoreset (0: no, 1: yes)\n", blockpathautoreset);
		fprintf(out, "%d\t;pondering (0: off, 1: on)\n", infopondering);
		fprintf(out, "%d\t;chekmate (0: normal, 1: vct search only, 2: vc2 search only)\n", infocheckmate);
		fprintf(out, "%d\t;hash autoclear (0: no, 1: yes)\n", hashautoclear);
		fprintf(out, "%d\t;toolbar postion (0: left, 1: right)\n", toolbarpos);
		fprintf(out, "%d\t;toolbar item number (<=32)\n", toolbarnum);
		fprintf(out, "%d\t;hotkey number (<=32)\n", hotkeynum);
		fclose(out);
	}
	for (i = 0; i < toolbarnum; i++)
	{
		sprintf(s, "function/toolbar%d.txt", i+1);
		if ((out = fopen(s, "w")) != NULL)
		{
			fprintf(out, "%d\n", toolbarlng[i]);
			fprintf(out, "%s\n", toolbaricon[i]);
			fprintf(out, "%s\n", toolbarcommand[i]);
			fclose(out);
		}
	}
	for (i = 0; i < hotkeynum; i++)
	{
		sprintf(s, "function/hotkey%d.txt", i + 1);
		if ((out = fopen(s, "w")) != NULL)
		{
			fprintf(out, "%d\n", hotkeykey[i]);
			fprintf(out, "%s\n", hotkeycommand[i]);
			fclose(out);
		}
	}
}

void yixin_quit()
{
	//stop_thinking(NULL, NULL);
	send_command("end\n");
	if (openbook != NULL) free(openbook);
	save_setting();
	if (clanguage != NULL)
	{
		int i;
		for (i = 0; i < 1024; i++)
		{
			if (clanguage[i] != NULL) free(clanguage[i]);
		}
		free(clanguage);
	}
	gtk_main_quit();
}

GdkPixbuf * copy_subpixbuf(GdkPixbuf *_src, int src_x, int src_y, int width, int height)
{
	GdkPixbuf *dst, *src;
	int sample, channels;
	gboolean alpha;
	GdkColorspace colorspace;
	
	guchar *pixels1, *pixels2;
	int rowstride1, rowstride2;
	int i, j;

	src = gdk_pixbuf_new_subpixbuf(_src, src_x, src_y, width, height);
	sample = gdk_pixbuf_get_bits_per_sample(src);
	channels = gdk_pixbuf_get_n_channels(src);
	alpha = gdk_pixbuf_get_has_alpha(src);
	colorspace = gdk_pixbuf_get_colorspace(src);
	dst = gdk_pixbuf_new(colorspace, alpha, sample, width, height);

	pixels1 = gdk_pixbuf_get_pixels(src);
	pixels2 = gdk_pixbuf_get_pixels(dst);

	rowstride1 = gdk_pixbuf_get_rowstride(src);
	rowstride2 = gdk_pixbuf_get_rowstride(dst);
	for(i=0; i<height; i++)
	{
		for(j=0; j<width; j++)
		{
			guchar *p1, *p2;
			p1 = pixels1 + i*rowstride1 + j*channels;
			p2 = pixels2 + i*rowstride2 + j*channels;
			memcpy(p2, p1, channels);
		}
	}
	g_object_unref(src);
	src = NULL;
	return dst;
}

gboolean key_press(GtkWidget *widget, GdkEventKey  *event, gpointer data)
{
	int i;
	if (event->state & GDK_CONTROL_MASK)
	{
		for (i = 0; i < hotkeynum; i++)
		{
			if (hotkeykeylis[hotkeykey[i]][0] & GDK_CONTROL_MASK)
			{
				if (event->keyval == hotkeykeylis[hotkeykey[i]][1])
				{
					hotkey_function(widget, (gpointer)i);
					return TRUE;
				}
			}
		}
	}
	else
	{
		for (i = 0; i < hotkeynum; i++)
		{
			if (!(hotkeykeylis[hotkeykey[i]][0] & GDK_CONTROL_MASK))
			{
				if (event->keyval == hotkeykeylis[hotkeykey[i]][1])
				{
					hotkey_function(widget, (gpointer)i);
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

void create_windowmain()
{
	GtkWidget *menubar, *menugame, *menuplayers, *menuview, *menuhelp, *menurule, *menulanguage;
	GtkWidget *menuitemgame, *menuitemplayers, *menuitemview, *menuitemhelp;
	GtkWidget *menuitemnewgame, *menuitemload, *menuitemsave, *menuitemrule, *menuitemsize, *menuitemopenbook, *menuitemquit,
		*menuitemrule1, *menuitemrule2, *menuitemrule3, *menuitemrule4, *menuitemrule5;
	//GtkWidget *menuitemnewrule[10]; //TODO
	GtkWidget *menuitemcomputerplaysblack, *menuitemcomputerplayswhite, *menuitemsettings;
	GtkWidget *menuitemlanguage, *menuitemenglish, *menuitemcustomlng[16] = { 0 }; //At most (16-1) different custom languages
	GtkWidget *menuitemnumeration, *menuitemlog, *menuitemanalysis;
	GtkWidget *menuitemabout;

	GtkToolItem *tools[MAX_TOOLBAR_ITEM];

	GtkWidget *textcommand;

	GdkPixbuf *pixbuf;

	GtkWidget *hbox[2], *vbox[2];

	int size, sample, channels, rowstride;
	gboolean alpha;
	GdkColorspace colorspace;
	GdkPixbuf *background;

	char lnglis[16][64];

	int i, j, k, l;

	windowmain = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(windowmain), FALSE); /* do not allow maximizing the window */
	g_signal_connect(GTK_OBJECT(windowmain), "destroy", G_CALLBACK(yixin_quit), NULL);
	gtk_widget_add_events(windowmain, GDK_BUTTON_PRESS_MASK); /* add the button clicking event */
	gtk_signal_connect(GTK_OBJECT(windowmain), "button_press_event", G_CALLBACK(on_button_press_windowmain), NULL);
	gtk_window_set_title(GTK_WINDOW(windowmain), "Yixin");
	tableboard = gtk_table_new(boardsizeh+1, boardsizew+1, TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(tableboard), 0); /* set the row distance between elements to be 0 */
	gtk_table_set_col_spacings(GTK_TABLE(tableboard), 0); /* set the column distance between elements to be 0 */

	pixbuf = gdk_pixbuf_new_from_file(piecepicname, NULL);
	size = gdk_pixbuf_get_height(pixbuf);
	sample = gdk_pixbuf_get_bits_per_sample(pixbuf);
	alpha = gdk_pixbuf_get_has_alpha(pixbuf);
	colorspace = gdk_pixbuf_get_colorspace(pixbuf);
	background = gdk_pixbuf_new_subpixbuf(pixbuf, size*14, 3, 1, 1);
	pixbufboard[0][0] = copy_subpixbuf(pixbuf, 0, 0, size, size);
	channels = gdk_pixbuf_get_n_channels(pixbufboard[0][0]);
	rowstride = gdk_pixbuf_get_rowstride(pixbufboard[0][0]);

	pixbufboard[1][0] = copy_subpixbuf(pixbuf, size, 0, size, size);
	for(i=2; i<=4; i++)
	{
		guchar *pixels1, *pixels2;

		pixbufboard[i][0] = gdk_pixbuf_new(colorspace, alpha, sample, size, size);
		pixels1 = gdk_pixbuf_get_pixels(pixbufboard[i-1][0]);
		pixels2 = gdk_pixbuf_get_pixels(pixbufboard[i][0]);
		for(j=0; j<size; j++)
		{
			for(k=0; k<size; k++)
			{
				guchar *p1, *p2;
				p1 = pixels1 + j*rowstride + k*channels;
				p2 = pixels2 + k*rowstride + (size-1-j)*channels;
				memcpy(p2, p1, channels);
			}
		}
	}
	pixbufboard[5][0] = copy_subpixbuf(pixbuf, size*2, 0, size, size);
	for(i=6; i<=8; i++)
	{
		guchar *pixels1, *pixels2, *pixels3;

		pixbufboard[i][0] = gdk_pixbuf_new(colorspace, alpha, sample, size, size);
		pixels1 = gdk_pixbuf_get_pixels(pixbufboard[i-1][0]);
		pixels2 = gdk_pixbuf_get_pixels(pixbufboard[i][0]);
		pixels3 = gdk_pixbuf_get_pixels(background);
		for(j=0; j<size; j++)
		{
			for(k=0; k<size; k++)
			{
				guchar *p1, *p2;
				if(size%2 == 1 || i!=7)
				{
					p1 = pixels1 + j*rowstride + k*channels;
					p2 = pixels2 + k*rowstride + (size-1-j)*channels;
				}
				else
				{
					p1 = pixels1 + ((j+1)%size)*rowstride + k*channels;
					p2 = pixels2 + k*rowstride + (size-1-j)*channels;
				}
				memcpy(p2, p1, channels);
			}
		}
	}
	for(i=0; i<=8; i++)
	{
		for(j=1; j<=2; j++)
		{
			GdkPixbuf *pbt;
			guchar *pixels1, *pixels2, *pixels3, *pixels4;
			pbt = copy_subpixbuf(pixbuf, size*(j+2), 0, size, size);
			pixbufboard[i][j] = gdk_pixbuf_new(colorspace, alpha, sample, size, size);
			pixels1 = gdk_pixbuf_get_pixels(pixbufboard[i][0]);
			pixels2 = gdk_pixbuf_get_pixels(pbt);
			pixels3 = gdk_pixbuf_get_pixels(background);
			pixels4 = gdk_pixbuf_get_pixels(pixbufboard[i][j]);
			for(k=0; k<size; k++)
			{
				for(l=0; l<size; l++)
				{
					guchar *p1, *p2, *p3, *p4;
					p1 = pixels1 + k*rowstride + l*channels;
					p2 = pixels2 + k*rowstride + l*channels;
					p3 = pixels3;
					p4 = pixels4 + k*rowstride + l*channels;
					if(memcmp(p2, p3, channels) != 0)
						memcpy(p4, p2, channels);
					else
						memcpy(p4, p1, channels);
				}
			}
			g_object_unref(pbt);
			pbt = NULL;
		}
		for(j=3; j<=4; j++)
		{
			GdkPixbuf *pbt1, *pbt2;
			guchar *pixels1, *pixels2, *pixels3, *pixels4, *pixels5;
			pbt1 = copy_subpixbuf(pixbuf, size*(j+4), 0, size, size);
			pbt2 = copy_subpixbuf(pixbuf, size*6, 0, size, size);
			pixbufboard[i][j] = gdk_pixbuf_new(colorspace, alpha, sample, size, size);
			pixels1 = gdk_pixbuf_get_pixels(pixbufboard[i][0]);
			pixels2 = gdk_pixbuf_get_pixels(pbt1);
			pixels3 = gdk_pixbuf_get_pixels(pbt2);
			pixels4 = gdk_pixbuf_get_pixels(background);
			pixels5 = gdk_pixbuf_get_pixels(pixbufboard[i][j]);
			for(k=0; k<size; k++)
			{
				for(l=0; l<size; l++)
				{
					guchar *p1, *p2, *p3, *p4, *p5;
					p1 = pixels1 + k*rowstride + l*channels;
					p2 = pixels2 + k*rowstride + l*channels;
					p3 = pixels3 + k*rowstride + l*channels;
					p4 = pixels4;
					p5 = pixels5 + k*rowstride + l*channels;
					if(memcmp(p3, p4, channels) != 0 || memcmp(p1, p4, channels) == 0)
						memcpy(p5, p2, channels);
					else
						memcpy(p5, p1, channels);
				}
			}
			g_object_unref(pbt1);
			pbt1 = NULL;
			g_object_unref(pbt2);
			pbt2 = NULL;
		}
		for(j=5; j<=6; j++)
		{
			GdkPixbuf *pbt;
			guchar *pixels1, *pixels2, *pixels3, *pixels4;
			pbt = copy_subpixbuf(pixbuf, size*5, 0, size, size);
			pixbufboard[i][j] = gdk_pixbuf_new(colorspace, alpha, sample, size, size);
			pixels1 = gdk_pixbuf_get_pixels(pixbufboard[i][j-2]);
			pixels2 = gdk_pixbuf_get_pixels(pbt);
			pixels3 = gdk_pixbuf_get_pixels(background);
			pixels4 = gdk_pixbuf_get_pixels(pixbufboard[i][j]);
			for(k=0; k<size; k++)
			{
				for(l=0; l<size; l++)
				{
					guchar *p1, *p2, *p3, *p4;
					p1 = pixels1 + k*rowstride + l*channels;
					p2 = pixels2 + k*rowstride + l*channels;
					p3 = pixels3;
					p4 = pixels4 + k*rowstride + l*channels;
					if(memcmp(p2, p3, channels) != 0)
						memcpy(p4, p2, channels);
					else
						memcpy(p4, p1, channels);
				}
			}
			g_object_unref(pbt);
			pbt = NULL;
		}
		for(j=7; j<=11; j++)
		{
			GdkPixbuf *pbt;
			guchar *pixels1, *pixels2, *pixels3, *pixels4;
			pbt = copy_subpixbuf(pixbuf, size*(j+2), 0, size, size);
			pixbufboard[i][j] = gdk_pixbuf_new(colorspace, alpha, sample, size, size);
			pixels1 = gdk_pixbuf_get_pixels(pixbufboard[i][0]);
			pixels2 = gdk_pixbuf_get_pixels(pbt);
			pixels3 = gdk_pixbuf_get_pixels(background);
			pixels4 = gdk_pixbuf_get_pixels(pixbufboard[i][j]);
			for(k=0; k<size; k++)
			{
				for(l=0; l<size; l++)
				{
					guchar *p1, *p2, *p3, *p4;
					p1 = pixels1 + k*rowstride + l*channels;
					p2 = pixels2 + k*rowstride + l*channels;
					p3 = pixels3;
					p4 = pixels4 + k*rowstride + l*channels;
					if(memcmp(p2, p3, channels) != 0)
						memcpy(p4, p2, channels);
					else
						memcpy(p4, p1, channels);
				}
			}
			g_object_unref(pbt);
			pbt = NULL;
		}
	}
	
	for(i=0; i<boardsizeh; i++)
	{
		for(j=0; j<boardsizew; j++)
		{
			if(i==0 && j==0) imgtypeboard[i][j] = 1;
			else if(i==0 && j==boardsizew-1) imgtypeboard[i][j] = 2;
			else if(i==boardsizeh-1 && j==boardsizew-1) imgtypeboard[i][j] = 3;
			else if(i==boardsizeh-1 && j==0) imgtypeboard[i][j] = 4;
			else if(i==0) imgtypeboard[i][j] = 5;
			else if(j==boardsizew-1) imgtypeboard[i][j] = 6;
			else if(i==boardsizeh-1) imgtypeboard[i][j] = 7;
			else if(j==0) imgtypeboard[i][j] = 8;
			else if(i==boardsizeh/2 && j==boardsizew/2) imgtypeboard[i][j] = 9;
			else if(i==boardsizeh/4 && j==boardsizew/4) imgtypeboard[i][j] = 9;
			else if(i==boardsizeh/4 && j==boardsizew-1-boardsizew/4) imgtypeboard[i][j] = 9;
			else if(i==boardsizeh-1-boardsizeh/4 && j==boardsizew/4) imgtypeboard[i][j] = 9;
			else if(i==boardsizeh-1-boardsizeh/4 && j==boardsizew-1-boardsizew/4) imgtypeboard[i][j] = 9;
			else imgtypeboard[i][j] = 0;
		}
	}
	for (i = 0; i < boardsizeh; i++)
	{
		char tlabel[3];
		sprintf(tlabel, "%d", boardsizeh - i);
		labelboard[0][i] = gtk_label_new(tlabel);
		gtk_table_attach_defaults(GTK_TABLE(tableboard), labelboard[0][i], boardsizew, boardsizew + 1, i, i + 1);
	}
	for (i = 0; i < boardsizew; i++)
	{
		char tlabel[3];
		sprintf(tlabel, "%c", 'A'+i);
		labelboard[1][i] = gtk_label_new(tlabel);
		gtk_table_attach_defaults(GTK_TABLE(tableboard), labelboard[1][i], i, i+1, boardsizeh, boardsizeh+1);
	}
	for(i=0; i<boardsizeh; i++)
	{
		for(j=0; j<boardsizew; j++)
		{
			if(imgtypeboard[i][j] <= 8)
				imageboard[i][j] = gtk_image_new_from_pixbuf(pixbufboard[imgtypeboard[i][j]][0]);
			else
				imageboard[i][j] = gtk_image_new_from_pixbuf(pixbufboard[0][1]);
			gtk_table_attach_defaults(GTK_TABLE(tableboard), imageboard[i][j], j, j+1, i, i+1);
		}
	}

	g_object_unref(background);
	background = NULL;
	g_object_unref(pixbuf);
	pixbuf = NULL;

	showsmallfont = (size < 24);

	menubar = gtk_menu_bar_new();
	menugame = gtk_menu_new();
	menuplayers = gtk_menu_new();
	menuview = gtk_menu_new();
	menuhelp = gtk_menu_new();
	menurule = gtk_menu_new();
	menulanguage = gtk_menu_new();

	menuitemgame = gtk_menu_item_new_with_label(language==0?"Game":_T(clanguage[59]));
	menuitemplayers = gtk_menu_item_new_with_label(language==0?"Players":_T(clanguage[60]));
	menuitemview = gtk_menu_item_new_with_label(language==0?"View":_T(clanguage[61]));
	menuitemhelp = gtk_menu_item_new_with_label(language==0?"Help":_T(clanguage[62]));
	menuitemnewgame = gtk_menu_item_new_with_label(language==0?"New":_T(clanguage[63]));
	menuitemload = gtk_menu_item_new_with_label(language==0?"Load":_T(clanguage[64]));
	menuitemsave = gtk_menu_item_new_with_label(language==0?"Save":_T(clanguage[65]));
	menuitemrule = gtk_menu_item_new_with_label(language==0?"Rule":_T(clanguage[66]));
	menuitemsize = gtk_menu_item_new_with_label(language==0?"Board Size":_T(clanguage[67]));
	menuitemopenbook = gtk_check_menu_item_new_with_label(language==0?"Use Openbook":_T(clanguage[68]));
	menuitemnumeration = gtk_check_menu_item_new_with_label(language==0?"Numeration":_T(clanguage[69]));
	menuitemlog = gtk_check_menu_item_new_with_label(language==0?"Log":_T(clanguage[70]));
	menuitemanalysis = gtk_check_menu_item_new_with_label(language==0?"Analysis":_T(clanguage[71]));
	menuitemlanguage = gtk_menu_item_new_with_label(language==0?"Language":_T(clanguage[72]));
	menuitemquit = gtk_menu_item_new_with_label(language==0?"Quit":_T(clanguage[73]));
	menuitemabout = gtk_menu_item_new_with_label(language==0?"About":_T(clanguage[74]));
	menuitemsettings = gtk_menu_item_new_with_label(language==0?"Settings":_T(clanguage[75]));
	
	menuitemrule1 = gtk_radio_menu_item_new_with_label(NULL, language==0?"Freestyle Gomoku":_T(clanguage[76]));
	menuitemrule2 = gtk_radio_menu_item_new_with_label(gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitemrule1)), language==0?"Standard Gomoku":_T(clanguage[77]));
	menuitemrule3 = gtk_radio_menu_item_new_with_label(gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitemrule1)), language==0?"Free Renju":_T(clanguage[78]));
	menuitemrule4 = gtk_radio_menu_item_new_with_label(gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitemrule1)), language==0?"RIF Opening Rule":_T(clanguage[79]));
	menuitemrule5 = gtk_radio_menu_item_new_with_label(gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitemrule1)), language==0?"Swap After First Move":_T(clanguage[80]));
	
	menuitemenglish = gtk_radio_menu_item_new_with_label(NULL, "English");
	for (i = 1; i < 16; i++)
	{
		FILE *in;
		char s[80];
		sprintf(s, "language\\%d.lng", i);
		if ((in = fopen(s, "r")) != NULL)
		{
			while (fgets(s, sizeof(s), in))
			{
				int l = strlen(s);
				int p;
				while (l > 0 && (s[l - 1] == '\n' || s[l - 1] == '\r'))
				{
					s[l - 1] = 0;
					l--;
				}
				if (l == 0) break;
				if (s[0] == ';') break;
				sscanf(s, "%d", &p);
				if (p == 84)
				{
					for (j = 0; j < l && s[j] != '='; j++);
					strcpy(lnglis[i], s + j + 1);
					break;
				}
			}
			fclose(in);
			menuitemcustomlng[i] = gtk_radio_menu_item_new_with_label(gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(menuitemenglish)), _T(lnglis[i]));
		}
	}
	
	switch(inforule)
	{
		case 0:
			if(specialrule == 0)
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemrule1), TRUE);
			else if(specialrule == 2)
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemrule5), TRUE);
			break;
		case 1: gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemrule2), TRUE); break;
		case 2:
			if(specialrule == 0)
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemrule3), TRUE);
			else if(specialrule == 1)
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemrule4), TRUE);
			break;
	}
	set_rule();

	if(language == 0)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemenglish), TRUE);
	else
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemcustomlng[language]), TRUE);

	if(useopenbook)
	{
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemopenbook), TRUE);
	}
	if(shownumber)
	{
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemnumeration), TRUE);
	}
	if(showlog)
	{
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemlog), TRUE);
	}
	if(showanalysis)
	{
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemanalysis), TRUE);
	}
	menuitemcomputerplaysblack = gtk_check_menu_item_new_with_label(language==0?"Computer Plays Black":_T(clanguage[81]));
	menuitemcomputerplayswhite = gtk_check_menu_item_new_with_label(language==0?"Computer Plays White":_T(clanguage[82]));
	if(computerside&1)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemcomputerplaysblack), TRUE);
	else
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemcomputerplaysblack), FALSE);
	if(computerside>>1)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemcomputerplayswhite), TRUE);
	else
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitemcomputerplayswhite), FALSE);
	change_side_menu(3, menuitemcomputerplaysblack);
	change_side_menu(4, menuitemcomputerplayswhite);

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitemgame), menugame);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitemplayers), menuplayers);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitemview), menuview);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitemhelp), menuhelp);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitemrule), menurule);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitemlanguage), menulanguage);

	gtk_menu_shell_append(GTK_MENU_SHELL(menurule), menuitemrule1);
	gtk_menu_shell_append(GTK_MENU_SHELL(menurule), menuitemrule2);
	gtk_menu_shell_append(GTK_MENU_SHELL(menurule), menuitemrule3);
	gtk_menu_shell_append(GTK_MENU_SHELL(menurule), menuitemrule4);
	gtk_menu_shell_append(GTK_MENU_SHELL(menurule), menuitemrule5);

	gtk_menu_shell_append(GTK_MENU_SHELL(menulanguage), menuitemenglish);
	for (i = 1; i < 16; i++)
	{
		if(menuitemcustomlng[i] != NULL)
			gtk_menu_shell_append(GTK_MENU_SHELL(menulanguage), menuitemcustomlng[i]);
	}

	gtk_menu_shell_append(GTK_MENU_SHELL(menugame), menuitemnewgame);
	gtk_menu_shell_append(GTK_MENU_SHELL(menugame), menuitemload);
	gtk_menu_shell_append(GTK_MENU_SHELL(menugame), menuitemsave);
	gtk_menu_shell_append(GTK_MENU_SHELL(menugame), menuitemrule);
	gtk_menu_shell_append(GTK_MENU_SHELL(menugame), menuitemsize);
	gtk_menu_shell_append(GTK_MENU_SHELL(menugame), menuitemopenbook);
	gtk_menu_shell_append(GTK_MENU_SHELL(menugame), menuitemquit);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuview), menuitemnumeration);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuview), menuitemlog);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuview), menuitemanalysis);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuview), menuitemlanguage);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuplayers), menuitemcomputerplaysblack);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuplayers), menuitemcomputerplayswhite);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuplayers), menuitemsettings);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuhelp), menuitemabout);

	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menuitemgame);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menuitemplayers);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menuitemview);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menuitemhelp);

	g_signal_connect(G_OBJECT(menuitemnewgame), "activate", G_CALLBACK(new_game), NULL);
	g_signal_connect(G_OBJECT(menuitemload), "activate", G_CALLBACK(show_dialog_load), windowmain);
	g_signal_connect(G_OBJECT(menuitemsave), "activate", G_CALLBACK(show_dialog_save), windowmain);
	g_signal_connect(G_OBJECT(menuitemopenbook), "activate", G_CALLBACK(use_openbook), NULL);
	g_signal_connect(G_OBJECT(menuitemnumeration), "activate", G_CALLBACK(view_numeration), NULL);
	g_signal_connect(G_OBJECT(menuitemlog), "activate", G_CALLBACK(view_log), NULL);
	g_signal_connect(G_OBJECT(menuitemanalysis), "activate", G_CALLBACK(view_analysis), NULL);
	g_signal_connect(G_OBJECT(menuitemquit), "activate", G_CALLBACK(yixin_quit), NULL);
	g_signal_connect(G_OBJECT(menuitemabout), "activate", G_CALLBACK(show_dialog_about), GTK_WINDOW(windowmain));
	g_signal_connect(G_OBJECT(menuitemsettings), "activate", G_CALLBACK(show_dialog_settings), GTK_WINDOW(windowmain));
	g_signal_connect(G_OBJECT(menuitemrule1), "activate", G_CALLBACK(change_rule), (gpointer)0);
	g_signal_connect(G_OBJECT(menuitemrule2), "activate", G_CALLBACK(change_rule), (gpointer)1);
	g_signal_connect(G_OBJECT(menuitemrule3), "activate", G_CALLBACK(change_rule), (gpointer)2);
	g_signal_connect(G_OBJECT(menuitemrule4), "activate", G_CALLBACK(change_rule), (gpointer)3);
	g_signal_connect(G_OBJECT(menuitemrule5), "activate", G_CALLBACK(change_rule), (gpointer)4);
	g_signal_connect(G_OBJECT(menuitemsize), "activate", G_CALLBACK(show_dialog_size), 0);
	g_signal_connect(G_OBJECT(menuitemcomputerplaysblack), "activate", G_CALLBACK(change_side), (gpointer)1);
	g_signal_connect(G_OBJECT(menuitemcomputerplayswhite), "activate", G_CALLBACK(change_side), (gpointer)2);
	g_signal_connect(G_OBJECT(menuitemenglish), "activate", G_CALLBACK(change_language), (gpointer)0);
	for (i = 1; i < 16; i++)
	{
		if(menuitemcustomlng[i] != NULL)
			g_signal_connect(G_OBJECT(menuitemcustomlng[i]), "activate", G_CALLBACK(change_language), (gpointer)i);
	}

	toolbar = gtk_toolbar_new();
	if(!showtoolbarboth)
		gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
	else
		gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_BOTH);
	if(toolbarpos)
		gtk_toolbar_set_orientation((GtkToolbar*)toolbar, GTK_ORIENTATION_HORIZONTAL);
	else
		gtk_toolbar_set_orientation((GtkToolbar*)toolbar, GTK_ORIENTATION_VERTICAL);

	for (i = 0; i < toolbarnum; i++)
	{
		tools[i] = gtk_tool_button_new_from_stock(toolbaricon[i]);
		gtk_tool_button_set_label(GTK_TOOL_BUTTON(tools[i]), _T(clanguage[toolbarlng[i]]));
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tools[i], -1);
		g_signal_connect(G_OBJECT(tools[i]), "clicked", G_CALLBACK(toolbar_function), (gpointer)i);
	}

	textlog = gtk_text_view_new();
	buffertextlog = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textlog));
	scrolledtextlog = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolledtextlog), textlog);
	gtk_widget_set_size_request(scrolledtextlog, 400, max(0, size*boardsizeh - 50 - (toolbarpos == 1 ? 50 : 0)));

	textcommand = gtk_text_view_new();
	buffertextcommand = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textcommand));
	scrolledtextcommand = gtk_scrolled_window_new(NULL, NULL);
	//gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolledtextcommand), textcommand);
	gtk_container_add(GTK_CONTAINER(scrolledtextcommand), textcommand);
	gtk_widget_set_size_request(scrolledtextcommand, 400, 50);
	g_signal_connect(textcommand, "key-release-event", G_CALLBACK(key_command), NULL);

	vbox[0] = gtk_vbox_new(FALSE, 0);
	if (toolbarpos == 1)
		gtk_box_pack_start(GTK_BOX(vbox[0]), toolbar, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(vbox[0]), scrolledtextlog, TRUE, TRUE, 3);
	gtk_box_pack_start(GTK_BOX(vbox[0]), scrolledtextcommand, FALSE, FALSE, 3);

	hbox[0] = gtk_hbox_new(FALSE, 0);
	if (toolbarpos == 0)
		gtk_box_pack_start(GTK_BOX(hbox[0]), toolbar, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(hbox[0]), tableboard, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(hbox[0]), vbox[0], FALSE, FALSE, 3);

	vboxwindowmain = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vboxwindowmain), menubar, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(vboxwindowmain), hbox[0], FALSE, FALSE, 3);
	//gtk_box_pack_start(GTK_BOX(vboxwindowmain), toolbar, FALSE, FALSE, 3);
	//gtk_box_pack_start(GTK_BOX(vboxwindowmain), tableboard, FALSE, FALSE, 3);

	gtk_container_add(GTK_CONTAINER(windowmain), vboxwindowmain);

	g_signal_connect(G_OBJECT(windowmain), "key-press-event", G_CALLBACK(key_press), NULL);

	gtk_widget_show_all(windowmain);
	
	//gtk_widget_hide(toolbar_acc);

	if(!showlog)
	{
		gtk_widget_hide(scrolledtextlog);
		gtk_widget_hide(scrolledtextcommand);
		gtk_widget_hide(toolbar);
	}
}

gboolean iochannelout_watch(GIOChannel *channel, GIOCondition cond, gpointer data)
{
	gchar *string;
	gsize size;
	int x, y;
	int i;
	char command[80];

	if(cond & G_IO_HUP)
	{
		g_io_channel_unref(channel);
		return FALSE;
	}
	do
	{
		g_io_channel_read_line(channel, &string, &size, NULL, NULL);
		
		if (commandmodel == 1)
		{
			print_log(string);
			g_free(string);
			continue;
		}

		for(i=0; i<(int)size; i++)
		{
			if(string[i] >= 'a' && string[i] <= 'z') string[i] = string[i] - 'a' + 'A';
		}
		if(strncmp(string, "OK", 2) == 0)
		{
			printf_log("%s", string);
			g_free(string);
			continue;
		}
		if(strncmp(string, "MESSAGE REALTIME", 16) == 0)
		{
			char *p = string + 16 + 1;
			if(*p == 'B') //"BEST"
			{
				p += 5;
				sscanf(p, "%d,%d", &y, &x);
				//printf_log("%d,%d\n", y, x);
				memset(boardbest, 0, sizeof(boardbest));
				boardbest[y][x] = 1;
				refresh_board();
			}
			else if(*p == 'V') //"VAL"
			{
				p += 4;
				sscanf(p, "%d", &bestval);
			}
			else if(*p == 'L') //"LOSE"
			{
				p += 5;
				sscanf(p, "%d,%d", &y, &x);
				boardlose[y][x] = 1;
				refresh_board();
			}
			else if(*p == 'P' && *(p+1) == 'V') //"PV"
			{
				p += 3;
				strcpy(bestline, p);
			}
			else if(*p == 'P' && *(p+1) == 'O') //"POS"
			{
				p += 4;
				sscanf(p, "%d,%d", &y, &x);
				boardpos[y][x] = 2;
				refresh_board();
			}
			else if(*p == 'R') //"REFRESH"
			{
				memset(boardpos, 0, sizeof(boardpos));
			}
			else if(*p == 'D') //"DONE"
			{
				p += 5;
				sscanf(p, "%d,%d", &y, &x);
				if(boardpos[y][x] == 2) boardpos[y][x] = 1;
			}
			g_free(string);
			continue;
		}
		if (strncmp(string, "MESSAGE INFO", 12) == 0)
		{
			char *p = string + 12 + 1;
			if (strncmp(p, "MAX_THREAD_NUM", 14) == 0) //MAX_THREAD_NUM
			{
				p += 15;
				sscanf(p, "%d", &maxthreadnum);
			}
			else if (strncmp(p, "MAX_HASH_SIZE", 13) == 0) //MAX_HASH_SIZE
			{
				p += 14;
				sscanf(p, "%d", &maxhashsize);
			}
			g_free(string);
			continue;
		}
		if(strncmp(string, "MESSAGE", 7) == 0)
		{
			printf_log("%s", string);
			g_free(string);
			continue;
		}
		if(strncmp(string, "DETAIL", 6) == 0)
		{
			printf_log("%s", string);
			g_free(string);
			continue;
		}
		if(strncmp(string, "DEBUG", 5) == 0)
		{
			printf_log("%s", string);
			g_free(string);
			continue;
		}
		if(strncmp(string, "ERROR", 5) == 0)
		{
			printf_log("%s", string);
			g_free(string);
			continue;
		}
		if(strncmp(string, "UNKNOWN", 7) == 0)
		{
			printf_log("%s", string);
			g_free(string);
			continue;
		}
		if(strncmp(string, "FORBID", 6) == 0)
		{
			memset(forbid, 0, sizeof(forbid));
			for(i=7; string[i] != '.'; i+=4)
			{
				y = (string[i] - '0')*10 + string[i+1] - '0';
				x = (string[i+2] - '0')*10 + string[i+3] - '0';
				forbid[y][x] = 1;
			}
			//printf_log("%s", string);
			g_free(string);
			refresh_board();
			continue;
		}
		sscanf(string, "%d,%d", &y, &x);
		if(isneedomit > 0)
		{
			g_free(string);
			isneedomit --;
		}
		else
		{
			isthinking = 0;
			timeused += (clock() - timestart) / (CLOCKS_PER_SEC / 1000);
			if(language)
			{
				printf_log(clanguage[83], timeoutmatch-timeused);
			}
			else
			{
				printf_log("Time Left: %dms", timeoutmatch-timeused);
			}
			printf_log("\n\n");
			if(blockautoreset)
			{
				send_command("yxblockreset\n");
				memset(boardblock, 0, sizeof(boardblock));
				refresh_board();
			}
			if (blockpathautoreset)
			{
				send_command("yxblockpathreset\n");
			}
			if(is_legal_move(y, x))
			{
				make_move(y, x);
			}
			else
			{
				isgameover = 1;
			}
			if(inforule == 2 && (computerside&1)==0)
			{
				sprintf(command, "yxshowforbid\n");
				send_command(command);
			}
			g_free(string);
			if(!isgameover && (((computerside&1)&&piecenum%2==0) || ((computerside&2)&&piecenum%2==1)))
			{
				isthinking = 1;
				timeused = 0;
				timestart = clock();
				sprintf(command, "INFO time_left %d\n", timeoutmatch-timeused);
				send_command(command);
				if(hashautoclear) send_command("yxhashclear\n");
				sprintf(command, "start %d %d\n", boardsizew, boardsizeh);
				send_command(command);
				sprintf(command, "board\n");
				send_command(command);
				for(i=0; i<piecenum; i++)
				{
					sprintf(command, "%d,%d,%d\n", movepath[i]/boardsizew,
						movepath[i]%boardsizew, piecenum%2==i%2 ? 1 : 2);
					send_command(command);
				}
				sprintf(command, "done\n");
				send_command(command);
			}
		}
	} while(g_io_channel_get_buffer_condition(channel) == G_IO_IN);
	return TRUE; /* TRUE means that the recalling function still runs later, while FALSE indicates it no longer runs */
}
gboolean iochannelerr_watch(GIOChannel *channel, GIOCondition cond, gpointer data)
{
	if(cond & G_IO_HUP)
	{
		g_io_channel_unref(channel);
		return FALSE ;
	}
	return TRUE;
}
int read_int_from_file(FILE *in)
{
	int flag = 0;
	int num = 0;
	char c;

	while(fscanf(in, "%c", &c) != EOF)
	{
		if(flag&1)
		{
			if(c=='\n') flag &= ~1;
		}
		else
		{
			if(c==';')
			{
				flag |= 1;
				if(flag & 2) return (flag&4)?-num:num;
			}
			else if(c<='9' && c>='0')
			{
				num *= 10;
				num += c-'0';
				flag |= 2;
			}
			else if(c=='-')
			{
				num = 0;
				flag |= 2;
				flag |= 4;
			}
			else
			{
				if(flag & 2) return (flag&4)?-num:num;
			}
		}
	}
	return (flag&4)?-num:num;
}
void load_setting(int def_boardsizeh, int def_boardsizew, int def_language, int def_toolbar)
{
	FILE *in;
	char s[1024];
	int t;
	int i;

	if((in = fopen("settings.txt", "r")) != NULL)
	{
		boardsizeh = read_int_from_file(in);
		if(def_boardsizeh >= 5 && def_boardsizeh <= MAX_SIZE)
		{
			boardsizeh = def_boardsizeh;
		}
		if(boardsizeh > MAX_SIZE || boardsizeh < 5) boardsizeh = 15;
		boardsizew = read_int_from_file(in);
		if (def_boardsizew >= 5 && def_boardsizew <= MAX_SIZE)
		{
			boardsizew = def_boardsizew;
		}
		if (boardsizew > MAX_SIZE || boardsizew < 5) boardsizew = 15;
		language = read_int_from_file(in);
		rboardsizeh = boardsizeh;
		rboardsizew = boardsizew;
		rlanguage = language;
		if(def_language >= 0 && def_language <= 1) language = def_language;
		if(language < 0) language = 0;
		inforule = read_int_from_file(in);
		if(inforule < 0 || inforule > 4) inforule = 0;
		if(inforule == 3)
		{
			inforule = 0;
			specialrule = 2;
		}
		if(inforule == 4)
		{
			inforule = 2;
			specialrule = 1;
		}
		useopenbook = read_int_from_file(in);
		if(useopenbook < 0 || useopenbook > 1) useopenbook = 1;
		computerside = 0;
		t = read_int_from_file(in);
		if(t == 1) computerside |= 1;
		t = read_int_from_file(in);
		if(t == 1) computerside |= 2;
		levelchoice = read_int_from_file(in);
		if(levelchoice < 0 || levelchoice > 8) levelchoice = 4;
		timeoutturn = read_int_from_file(in) * 1000;
		if (timeoutturn == 0) timeoutturn = 100;
		if(timeoutturn < 0 || timeoutturn > 2000000) timeoutturn = 10000;
		timeoutmatch = read_int_from_file(in) * 1000;
		if(timeoutmatch <= 0 || timeoutmatch > 1000000000) timeoutmatch = 2000000;
		maxdepth = read_int_from_file(in);
		if(maxdepth < 2 || maxdepth > boardsizeh*boardsizew) maxdepth = boardsizeh*boardsizew;
		maxnode = read_int_from_file(in);
		if(maxnode < 1000 || maxnode > 1000000000) maxnode = 1000000000;
		cautionfactor = read_int_from_file(in);
		if(cautionfactor < 0 || cautionfactor > CAUTION_NUM) cautionfactor = 1;
		showtoolbarboth = read_int_from_file(in);
		if(def_toolbar >= 0 && def_toolbar <= 1) showtoolbarboth = def_toolbar;
		if(showtoolbarboth < 0 || showtoolbarboth > 1) showtoolbarboth = 1;
		showlog = read_int_from_file(in);
		if(showlog < 0 || showlog > 1) showlog = 1;
		shownumber = read_int_from_file(in);
		if(shownumber < 0 || shownumber > 1) shownumber = 1;
		showanalysis = read_int_from_file(in);
		if(showanalysis < 0 || showanalysis > 1) showanalysis = 1;
		showwarning = read_int_from_file(in);
		if(showwarning < 0 || showwarning > 1) showwarning = 0;
		blockautoreset = read_int_from_file(in);
		if(blockautoreset < 0 || blockautoreset > 1) blockautoreset = 0;
		threadnum = read_int_from_file(in);
		if(threadnum < 1 /*|| threadnum > maxthreadnum*/) threadnum = 1;
		hashsize = read_int_from_file(in);
		if(hashsize < 0 /*|| hashsize > maxhashsize*/) hashsize = 19;
		threadsplitdepth = read_int_from_file(in);
		if(threadsplitdepth < MIN_SPLIT_DEPTH || threadsplitdepth > MAX_SPLIT_DEPTH) threadsplitdepth = 8;
		blockpathautoreset = read_int_from_file(in);
		if (blockpathautoreset < 0 || blockpathautoreset > 1) blockpathautoreset = 0;
		infopondering = read_int_from_file(in);
		if (infopondering < 0 || infopondering > 1) infopondering = 0;
		infocheckmate = read_int_from_file(in);
		if (infocheckmate < 0 || infocheckmate > 2) infocheckmate = 0;
		hashautoclear = read_int_from_file(in);
		if (hashautoclear < 0 || hashautoclear > 1) hashautoclear = 0;
		toolbarpos = read_int_from_file(in);
		if (toolbarpos < 0 || toolbarpos > 1) toolbarpos = 1;
		toolbarnum = read_int_from_file(in);
		if (toolbarnum < 0 || toolbarnum > MAX_TOOLBAR_ITEM) toolbarnum = 6;
		hotkeynum = read_int_from_file(in);
		if (hotkeynum < 0 || hotkeynum > MAX_HOTKEY_ITEM) hotkeynum = 6;
		fclose(in);
	}
	for (i = 0; i < toolbarnum; i++)
	{
		sprintf(s, "function/toolbar%d.txt", i+1);
		if ((in = fopen(s, "r")) != NULL)
		{
			int j = 0;
			char icon[80];
			fscanf(in, "%d", &toolbarlng[i]);
			fscanf(in, "%s", &icon);
			toolbaricon[i] = strdup(icon);

			while (fgets(toolbarcommand[i] + j, MAX_TOOLBAR_COMMAND_LEN, in))
			{
				j += strlen(toolbarcommand[i] + j);
				while (j > 0 && (toolbarcommand[i][j - 1] == '\n' || toolbarcommand[i][j - 1] == '\r')) j--;
				toolbarcommand[i][j] = '\n';
				j++;
				toolbarcommand[i][j] = 0;
			}
			fclose(in);
		}
	}
	for (i = 0; i < hotkeynum; i++)
	{
		sprintf(s, "function/hotkey%d.txt", i + 1);
		if ((in = fopen(s, "r")) != NULL)
		{
			int j = 0;
			char icon[80];
			fscanf(in, "%d", &hotkeykey[i]);

			while (fgets(hotkeycommand[i] + j, MAX_HOTKEY_COMMAND_LEN, in))
			{
				j += strlen(hotkeycommand[i] + j);
				while (j > 0 && (hotkeycommand[i][j - 1] == '\n' || hotkeycommand[i][j - 1] == '\r')) j--;
				hotkeycommand[i][j] = '\n';
				j++;
				hotkeycommand[i][j] = 0;
			}
			fclose(in);
		}
	}
	sprintf(s, "piece_%d.bmp", max(boardsizeh, boardsizew));
	if((in = fopen(s, "rb")) != NULL)
	{
		strcpy(piecepicname, s);
		fclose(in);
	}
	piecenum = 0;
	memset(movepath, -1, sizeof(movepath));

	clanguage = (char *)malloc(1024 * sizeof(char*));
	for (i = 0; i < 1024; i++) clanguage[i] = NULL;

	sprintf(s, "language\\%d.lng", language);
	if ((in = fopen(s, "r")) != NULL)
	{
		while (fgets(s, sizeof(s), in))
		{
			int l = strlen(s);
			int p;
			while (l > 0 && (s[l - 1] == '\n' || s[l - 1] == '\r'))
			{
				s[l - 1] = 0;
				l--;
			}
			if (l == 0) continue;
			if (s[0] == ';') continue;
			sscanf(s, "%d", &p);
			for (i = 0; i < l && s[i] != '='; i++);
			clanguage[p] = strdup(s + i + 1);
		}
		fclose(in);
	}
	else
	{
		language = 0;
	}
}
void load_engine()
{
#ifdef G_OS_WIN32
	gchar *argv[] = {"engine.exe", NULL};
#else
	gchar *argv[] = {"./engine", NULL};
#endif
	GPid pid;
	gint in, out, err;
	gboolean ret;
	GError *error = NULL;

	ret = g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD,
		NULL, NULL, &pid, &in, &out, &err, &error);
	if(!ret)
	{
		FILE *out;
		out = fopen("ERROR.txt", "w");
		fprintf(out, "%s\n", error->message);
		fclose(out);
		g_error_free(error);

		g_error("Cannot load engine!");
		return;
	}
#ifdef G_OS_WIN32
	iochannelin = g_io_channel_win32_new_fd(in);
    iochannelout = g_io_channel_win32_new_fd(out);
    iochannelerr = g_io_channel_win32_new_fd(err);
#else
	iochannelin = g_io_channel_unix_new(in);
    iochannelout = g_io_channel_unix_new(out);
    iochannelerr = g_io_channel_unix_new(err);
#endif
	g_io_add_watch(iochannelout, G_IO_IN | G_IO_PRI | G_IO_HUP, (GIOFunc)iochannelout_watch, NULL);
	//g_io_add_watch_full(iochannelout, G_PRIORITY_HIGH, G_IO_IN | G_IO_HUP, (GIOFunc)iochannelout_watch, NULL, NULL);
    g_io_add_watch(iochannelerr, G_IO_IN | G_IO_PRI | G_IO_HUP, (GIOFunc)iochannelerr_watch, NULL);
}
void init_engine()
{
	char command[80];
	send_command("info show_detail 1\n");
	send_command("yxshowinfo\n");
	sprintf(command, "START %d %d\n", boardsizew, boardsizeh);
	send_command(command);
	set_level(levelchoice);
	set_cautionfactor(cautionfactor);
	set_threadnum(threadnum);
	set_hashsize(hashsize);
	set_pondering(infopondering);
	set_checkmate(infocheckmate);
}
int main(int argc, char** argv)
{
	static GOptionEntry options[] =
	{
		{ "size", 's', 0, G_OPTION_ARG_INT, NULL,
			"Board size to use", "Integer"
		},
		{ "lang", 'l', 0, G_OPTION_ARG_INT, NULL,
			"language", "Integer"
		},
		{ "toolbar", '\0', 0, G_OPTION_ARG_INT, NULL,
			"Tool bar style", "Integer"
		},
		{
			NULL
		},
	};
	GError *error = NULL;
	gint boardsizeh = -1;
	gint boardsizew = -1;
	gint language = -1;
	gint toolbar = -1;

	options[0].arg_data = &boardsizeh;
	options[1].arg_data = &boardsizew;
	options[2].arg_data = &language;
	options[3].arg_data = &toolbar;

	gtk_init_with_args(&argc, &argv, NULL, options, NULL, &error);
	srand((unsigned)time(NULL));
	load_setting(boardsizeh, boardsizew, language, toolbar);
	load_engine();
	init_engine();
	gtk_window_set_default_icon(gdk_pixbuf_new_from_file("icon.ico", NULL)); /* set the default icon for all windows */
	create_windowmain();
	show_welcome();
	gtk_main();
	return 0;
}
