#pragma pack (1)
#include<stdio.h>
#include<stdlib.h>
#define M (1<<27)
#define RS (20000000)
#define SIZE 20
#define OUT_INFO 0
typedef __int64 I64;
I64 Zobrist[SIZE][SIZE][3];
I64 ZobKey;
FILE *bookfile, *out;
char bookdata[M];
int booklen;
char libname[80];
int p;
typedef struct
{
	I64 zobkey;
	char x;
} REC;
REC rec[RS];
int prec;
int cmp(const void *a, const void *b)
{
	if(((REC*)a)->zobkey > ((REC*)b)->zobkey) return 1;
	if(((REC*)a)->zobkey < ((REC*)b)->zobkey) return -1;
	return 0;
}
/*
注释 0x8
棋盘文字 0x1
标记 0x10
无儿子 0x40
有兄弟 0x80
*/
int seq[SIZE*SIZE];
int status[SIZE*SIZE];
void convert()
{
	int s;
	int i;
	int depth;
	prec = 0;
	
	depth = 0;
	p ++;
	while(p < booklen)
	{
		if(bookdata[p-1] == 0 && bookdata[p] == 0) break;
		p ++;
	}
	p ++;
	while(p < booklen)
	{
		if((p & 0xFFFF) == 0) printf("\b\b\b\b\b\b\b%d%%", (int)((I64)p*100/booklen));
		if(depth < 0) break;
		if(status[depth])
		{
			status[depth] = 0;
			depth --;
			continue;
		}
		seq[depth] = bookdata[p];
		
		ZobKey = 0;
		for(i=0; i<=depth; i++)
		{
			ZobKey ^= Zobrist[(seq[i]>>4)&15][(seq[i]&15)-1][i%2];
		}
		rec[prec].zobkey = ZobKey;
		rec[prec].x = 0;
		if(OUT_INFO) fprintf(out, "%I64d ", ZobKey);
		//for(i=0; i<=depth; i++) fprintf(out, "%d,%d ", (seq[i]>>4)&15, (seq[i]&15)-1);
		p ++;
		s = bookdata[p];
		status[depth] = !(s & 0x80);
		if((s & 0x09) == 0x09)
		{
			p += 4;
			if(OUT_INFO) fprintf(out, "{");
			while(p < booklen && bookdata[p] != 0)
			{
				if(bookdata[p] != '\n' && bookdata[p] != '\r')
				{
					if(OUT_INFO) fprintf(out, "%c", bookdata[p]);
					rec[prec].x = bookdata[p];
				}
				p ++;
			}
			if(OUT_INFO) fprintf(out, "}");
			p ++;
			if(OUT_INFO) fprintf(out, " {");
			while(p < booklen && bookdata[p] != 0)
			{
				if(bookdata[p] != '\n' && bookdata[p] != '\r')
					if(OUT_INFO) fprintf(out, "%c", bookdata[p]);
				p ++;
			}
			if(OUT_INFO) fprintf(out, "}");
			while(p < booklen && bookdata[p] == 0)
			{
				p ++;
			}
		}
		else if(s & 0x01)
		{
			p += 3;
			if(OUT_INFO) fprintf(out, "{");
			while(p < booklen && bookdata[p] != 0)
			{
				if(bookdata[p] != '\n' && bookdata[p] != '\r')
				{
					if(OUT_INFO) fprintf(out, "%c", bookdata[p]);
					rec[prec].x = bookdata[p];
				}
				p ++;
			}
			if(OUT_INFO)
			{
				fprintf(out, "}");
				fprintf(out, " {");
				fprintf(out, "}");
			}
			while(p < booklen && bookdata[p] == 0)
			{
				p ++;
			}
		}
		else if(s & 0x08)
		{
			p += 2;
			if(OUT_INFO)
			{
				fprintf(out, "{");
				fprintf(out, "}");
				fprintf(out, " {");
			}
			while(p < booklen && bookdata[p] != 0)
			{
				if(bookdata[p] != '\n' && bookdata[p] != '\r')
					if(OUT_INFO) fprintf(out, "%c", bookdata[p]);
				p ++;
			}
			if(OUT_INFO) fprintf(out, "}");
			while(p < booklen && bookdata[p] == 0)
			{
				p ++;
			}
		}
		else
		{
			p ++;
			if(OUT_INFO) fprintf(out, "{} {}");
		}
		if(s & 0x10)
		{
			if(OUT_INFO) fprintf(out, " {f}");
		}
		else
		{
			if(OUT_INFO) fprintf(out, " {}");
		}
		if(OUT_INFO) fprintf(out, "\n");
		//if(depth <= 5)
		{
			if(rec[prec].x >= 'A' && rec[prec].x <= 'G') prec ++;
			else if(rec[prec].x == 'h') prec ++;
			else if(rec[prec].x >= '0' && rec[prec].x <= '9') prec ++;
			else if(rec[prec].x >= '!') prec ++;
			else if(rec[prec].x == 'a') prec ++;
		}
		if(s & 0x40) continue;
		depth ++;
	}
}
void solve()
{
	int i, j;
	bookfile = fopen(libname, "rb");
	booklen = fread(bookdata, sizeof(char), M, bookfile);
	fclose(bookfile);
	if(OUT_INFO) out = fopen("info.txt", "w");
	printf("size = %d\n", booklen);
	convert();
	if(OUT_INFO) fclose(out);
	qsort(rec, prec, sizeof(REC), cmp);
	for(j=0,i=1; i<prec; i++)
	{
		if(rec[j].zobkey != rec[i].zobkey)
		{
			j ++;
			rec[j] = rec[i];
		}
	}
	prec = j+1;
	out = fopen("opening.dat", "wb");
	fwrite(rec, sizeof(REC), prec, out);
	fclose(out);
	printf("after merging: %d\n", prec);
	/*
	out = fopen("opening.txt", "w");
	for(i=0; i<prec; i++)
	{
		fprintf(out, "%I64d %c\n", rec[i].zobkey, rec[i].x);
	}
	fclose(out);
	*/
	
}
void BuildZobrist()
{
	int i, j, k;
	FILE *in;
	in = fopen("zobrist.txt", "r");
	for(i=0; i<SIZE; i++)
	{
		for(j=0; j<SIZE; j++)
		{
			for(k=0; k<3; k++)
			{
				fscanf(in, "%I64d ", &Zobrist[i][j][k]);
			}
		}
	}
	fclose(in);
}
int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("ERROR\n");
		printf("\nDONE\nPress any key to exit...");
		getchar();
		return 0;
	}
	strcpy(libname, argv[1]);
	BuildZobrist();
	solve();
	printf("\nDONE\nPress any key to exit...");
	getchar();
	return 0;
}
