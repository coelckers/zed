#pragma once


#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif




// rect_t: A rectangle
struct rect_t
{
	wxPoint tl, br;

	// Constructors
	rect_t() {  }
	rect_t(wxPoint TL, wxPoint BR) : tl(TL), br(BR) { }
	rect_t(int x1, int y1, int x2, int y2) : tl(x1, y1), br(x2, y2) {}

	rect_t(int x, int y, int width, int height, int align)
	{
		if (align == 0) // Top-left
		{
			tl = wxPoint(x, y);
			br = wxPoint(x + width, y + height);
		}

		if (align == 1) // Centered
		{
			tl = wxPoint(x - (width / 2), y - (height / 2));
			br = wxPoint(x + (width / 2), y + (height / 2));
		}
	}

	// Functions
	void set(wxPoint TL, wxPoint BR)
	{
		tl=TL;
		br=BR;
	}

	void set(int x1, int y1, int x2, int y2)
	{
		tl.x=x1;
		tl.y=y1;
		br.x=x2;
		br.y=y2;
	}

	void set(rect_t rect)
	{
		tl=rect.tl;
		br=rect.br;
	}

	int x1() { return tl.x; }
	int y1() { return tl.y; }
	int x2() { return br.x; }
	int y2() { return br.y; }

	int left()		{ return min(tl.x, br.x); }
	int top()		{ return min(tl.y, br.y); }
	int right()		{ return max(br.x, tl.x); }
	int bottom()	{ return max(br.y, tl.y); }

	int width() { return br.x - tl.x; }
	int height() { return br.y - tl.y; }

	int awidth() { return max(br.x, tl.x) - min(tl.x, br.x); }
	int aheight() { return max(br.y, tl.y) - min(tl.y, br.y); }

	void resize(int x, int y)
	{
		if (x1() < x2())
		{
			tl.x -= x;
			br.x += x;
		}
		else
		{
			tl.x += x;
			br.x -= x;
		}

		if (y1() < y2())
		{
			tl.y -= y;
			br.y += y;
		}
		else
		{
			tl.y += y;
			br.y -= y;
		}
	}

	double length()
	{
		double dist_x = x2() - x1();
		double dist_y = y2() - y1();

		return sqrt(dist_x * dist_x + dist_y * dist_y);
	}
};

// Yeah I know I could easily replace these with std::vectors, and I have where I can,
// but the line drawing stuff I really don't want to go though again :P
struct pointlist_t
{
	wxPoint	*points;
	DWORD		n_points;

	pointlist_t() { points = NULL; n_points = 0; }
	~pointlist_t() { if (points) free(points); }

	bool exists(wxPoint point)
	{
		for (DWORD n = 0; n < n_points; n++)
		{
			if (points[n].x == point.x &&
				points[n].y == point.y)
				return true;
		}

		return false;
	}

	bool add(wxPoint point, bool no_dup)
	{
		bool dup = exists(point);

		if (no_dup && dup)
			return false;

		n_points++;
		points = (wxPoint *)realloc(points, n_points * sizeof(wxPoint));
		points[n_points - 1] = point;

		if (dup)
			return false;
		else
			return true;
	}

	int find_index(wxPoint point)
	{
		for (DWORD n = 0; n < n_points; n++)
		{
			if (points[n].x == point.x &&
				points[n].y == point.y)
				return n;
		}

		return -1;
	}

	void remove(wxPoint point)
	{
		int n = find_index(point);

		if (n == -1)
			return;

		for (DWORD i = n; i < n_points - 1; i++)
			points[i] = points[i + 1];

		n_points--;
		points = (wxPoint *)realloc(points, n_points * sizeof(wxPoint));
	}

	void clear()
	{
		n_points = 0;
		free(points);
		points = NULL;
	}

	wxPoint get_last()
	{
		if (n_points > 0)
			return points[n_points - 1];
		else
			return wxPoint(0, 0);
	}

	const wxPoint & operator[](unsigned index)
	{
		return points[index];
	}
};

struct numlist_t
{
	int		*numbers;
	DWORD	n_numbers;

	numlist_t() { numbers = NULL; n_numbers = 0; }
	~numlist_t() { if (numbers) free(numbers); }

	bool exists(int num)
	{
		for (DWORD n = 0; n < n_numbers; n++)
		{
			if (numbers[n] == num)
				return true;
		}

		return false;
	}

	bool add(int num, bool no_dup)
	{
		if (no_dup && exists(num))
			return false;

		n_numbers++;
		numbers = (int *)realloc(numbers, n_numbers * sizeof(int));
		numbers[n_numbers - 1] = num;

		return true;
	}

	void add_numlist(numlist_t *list, bool no_dup)
	{
		for (DWORD a = 0; a < list->n_numbers; a++)
			add(list->numbers[a], no_dup);
	}

	int find_index(int num)
	{
		for (DWORD n = 0; n < n_numbers; n++)
		{
			if (numbers[n] == num)
				return n;
		}

		return -1;
	}

	int find_index_last(int num)
	{
		for (int n = n_numbers - 1; n >= 0; n--)
		{
			if (numbers[n] == num)
				return n;
		}

		return -1;
	}

	void remove(int num)
	{
		int n = find_index_last(num);

		if (n == -1)
			return;

		for (DWORD i = n; i < n_numbers - 1; i++)
			numbers[i] = numbers[i + 1];

		n_numbers--;
		numbers = (int *)realloc(numbers, n_numbers * sizeof(int));
	}

	void remove_last()
	{
		n_numbers--;
		numbers = (int *)realloc(numbers, n_numbers * sizeof(int));
	}

	void clear()
	{
		n_numbers = 0;
		free(numbers);
		numbers = NULL;
	}

	int get_last()
	{
		if (n_numbers > 0)
			return numbers[n_numbers - 1];
		else
			return 0;
	}
};
