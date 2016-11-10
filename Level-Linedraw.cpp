// Line drawing code from SLADE
//


#include "stdafx.h"
#include "tarray.h"
#include "linedraw.h"
#include "level.h"

#pragma warning(disable:4018)

pointlist_t ldraw_points;	// Points drawn (used to create verts & lines
numlist_t ldraw_lines;		// Lines drawn
numlist_t ldraw_verts;		// Vertices drawn

//static TArray<CPoint> linedrawlist;
static bool lastpointvalid;
static wxPoint lastpoint;

#define	LINE_IMPASSIBLE		0x0001
#define	LINE_TWOSIDED		0x0004

// distance: Returns the distance between two points
// ---------------------------------------------- >>
double distance(double x1, double y1, double x2, double y2)
{
	double dist_x = x2 - x1;
	double dist_y = y2 - y1;

	return sqrt(dist_x * dist_x + dist_y * dist_y);
}

// midpoint: Returns the mid point of a line between two points
// --------------------------------------------------------- >>
static wxPoint midpoint(wxPoint p1, wxPoint p2)
{
	return wxPoint(p1.x+ (p2.x-p1.x)/2, p1.y+ (p2.y-p1.y)/2);
}

// distance_to_line: Finds the shortest distance from a vertex to a line
// ------------------------------------------------------------------ >>
// (code borrowed & slightly modified from Doom Builder, by CodeImp)
double distance_to_line(double x1, double y1, double x2, double y2, double vx, double vy)
{
	double u;

	// Get length of linedef
	double ld = distance(x1, y1, x2, y2);

	// Check if not zero length
	if(ld)
	{
		// Get the intersection offset
		u = double((vx - x1) * (x2 - x1) + (vy - y1) * (y2 - y1)) / (ld * ld);

		// Limit intersection offset to the line
		double lbound = 1 / ld;
		double ubound = 1 - lbound;
		if(u < lbound) u = lbound;
		if(u > ubound) u = ubound;
	}
	else
	{
		u = 0;
	}

	// Calculate intersection point
	double ix = double(x1) + u * double(x2 - x1);
	double iy = double(y1) + u * double(y2 - y1);

	// Return distance between intersection and point
	// which is the shortest distance to the line
	return distance(double(vx), double(vy), ix, iy);
}

// determine_line_side: Determines what side of a line a certain point is on
// ---------------------------------------------------------------------- >>
bool determine_line_side(float x1, float y1, float x2, float y2, float x, float y)
{
	float side = ((-y + y1) * (x2 - x1)) - ((x - x1) * (-y2 + y1));

	if (side > 0)
		return true;
	else
		return false;
}


// bounding rectangle of a linedef
// --------------------------------------------------------- >>
rect_t CLevel::l_getrect(int l)	
{ 
	return rect_t(int(StVt(l)->X()-0.5), int(StVt(l)->Y()-0.5), 
				  int(EnVt(l)->X()+0.5), int(EnVt(l)->Y()+0.5)); 
}


// 
// --------------------------------------------------------- >>
int	CLevel::v_getvertatpoint(wxPoint point)
{
	if (NumVertices() == 0)
		return -1;

	for (int v = NumVertices() - 1; v >= 0; v--)
	{
		CVertex * vt = GetVertex(v);
		if (int(vt->X()) == point.x && int(vt->Y()) == point.y)
			return v;
	}

	return -1;
}

void CLevel::v_getattachedlines(int v, numlist_t* list)
{
	for (int l = 0; l < NumLines(); l++)
	{
		CLine * ln=GetLine(l);
		if (ln->line.Start == v || ln->line.End == v)
			list->add(l, false);
	}
}

wxPoint CLevel::v_getpoint(int v)	
{ 
	CVertex * vt=GetVertex(v);
	return wxPoint(int(vt->X()), int(vt->Y()));			
}

int CLevel::l_split(int l, int vertex)
{
	int line = InsertLineDef(GetLine(l));
	GetLine(l)->line.End = GetLine(line)->line.Start = vertex;
	return line;
}

int CLevel::add_line(int v1, int v2)
{
	int line = InsertLineDef(NULL);
	CLine * ln = GetLine(line);

	ln->line.Start=v1;
	ln->line.End=v2;
	return line;
}

bool CLevel::v_isattached_sector(int v)
{
	for (DWORD l = 0; l < NumLines(); l++)
	{
		if (StVtNo(l) == v || EnVtNo(l) == v)
		{
			if (FrontSecNo(l) != -1 || BackSecNo(l) != -1)
				return true;
		}
	}

	return false;
}

void CLevel::l_setsector(int l, int side, int sector)
{
	CLine * ln = GetLine(l);
	if (side == 1)
	{
		ln->sides[0].sector=sector;
	}
	else
	{
		/*
		if (sector == -1)
		{
			ln->line.flags&=~LINE_TWOSIDED;
			ln->line.flags|=LINE_IMPASSIBLE;
		}
		else
		{
			ln->line.flags|=LINE_TWOSIDED;
			ln->line.flags&=~LINE_IMPASSIBLE;
			strncpy(ln->sides[0].texNormal,"-",8);
		}
		*/
		ln->sides[1].sector=sector;
	}
}

// get_nearest_line_2: Gets the nearest line to a point
// ------------------------------------------------- >>
int CLevel::get_nearest_line_2(double x, double y)
{
	double min_dist = -1;
	int line = -1;

	for (unsigned l = 0; l < NumLines(); l++)
	{
		rect_t r = l_getrect(l);

		double dist = distance_to_line(r.x1(), r.y1(), r.x2(), r.y2(), x, y);

		if (min_dist == -1 && FrontSecNo(l) != -1)
		{
			min_dist = dist;
			line = l;
		}
		else if (dist < min_dist && FrontSecNo(l) != -1)
		{
			min_dist = dist;
			line = l;
		}
	}

	return line;
}

// determine_line_side: Determines what side of a line a certain point is on
// ---------------------------------------------------------------------- >>
bool CLevel::determine_line_side(int line, float x, float y)
{
	rect_t r;
	r.set(l_getrect(line));
	r.tl.y = -r.tl.y;
	r.br.y = -r.br.y;

	float side = (-y - (float)r.y1()) * float(r.x2() - r.x1()) - (x - (float)r.x1()) * float(r.y2() - r.y1());

	if (side > 0)
		return true;
	else
		return false;
}


// determine_sector: Determines what sector a point lies in
// (returns -1 if point is in the 'void')
int CLevel::determine_sector(double x, double y)
{
	//draw_text(16, 16, COL_WHITE, 0, "%d %d", x, y);
	int line = get_nearest_line_2(x, y);

	if (line == -1)
		return -1;

	if (determine_line_side(line, x, y))
		return FrontSecNo(line);
	else
		return BackSecNo(line);
}

// check_vertex_split: Checks if a vertex is close enough to a line to split it
// ------------------------------------------------------------------------- >>
int CLevel::check_vertex_split(DWORD vertex)
{
	int split_line = -1;
	CVertex * v=GetVertex(vertex);
	double vx = v->X();
	double vy = v->Y();

	if (GetGridSizeX() == 1 && GetGridSizeY() == 1)
		return -1;

	for (int l = 0; l < NumLines(); l++)
	{
		rect_t r = l_getrect(l);

		if (distance_to_line(r.x1(), r.y1(), r.x2(), r.y2(), vx, vy) < 2
			&& StVtNo(l) != vertex && EnVtNo(l) != vertex)
			return l;
	}

	return -1;
}

// check_vertex_split: Checks if a point is close enough to a line to split it
// ------------------------------------------------------------------------ >>
int CLevel::check_vertex_split(wxPoint p)
{
	return check_vertex_split(v_getvertatpoint(p));
}



void CLevel::v_merge(int v1, int v2)
{
	for (int l = 0; l < NumLines(); l++)
	{
		CLine * ln = GetLine(l);
		if (ln->line.Start == v1) ln->line.Start = v2;
		else if (ln->line.End == v1) ln->line.End = v2;
	}

	DeleteOneVertex(v1);
}

void CLevel::v_mergespot(int x, int y)
{
	//numlist_t merge;
	TArray<int> merge;

	for (DWORD v = 0; v < NumVertices(); v++)
	{
		CVertex * vt = GetVertex(v);
		if (int(vt->X()) == x && int(vt->Y()) == y)
			merge.Push(v);
	}

	for (DWORD v = 1; v < merge.Size(); v++)
		v_merge(merge[v], merge[0]);
}

// merge_verts: Merges all vertices in the map
// ---------------------------------------- >>
void CLevel::merge_verts()
{
	for (DWORD v = 0; v < NumVertices(); v++)
	{
		CVertex * vt = GetVertex(v);
		v_mergespot(vt->X(), vt->Y());
	}

}

bool CLevel::v_checkspot(int x, int y)
{
	for (DWORD v = 0; v <NumVertices() ; v++)
	{
		CVertex * vt = GetVertex(v);
		if (int(vt->X()) == x && int(vt->Y()) == y)
			return false;
	}

	return true;
}

// get_side_sector: Attempts to find what sector a line's side is in
// -------------------------------------------------------------- >>
int CLevel::get_side_sector(int line, int side)
{
	rect_t linerect = l_getrect(line);
	wxPoint mid = midpoint(linerect.tl, linerect.br);
	wxPoint vec(linerect.x2() - linerect.x1(), linerect.y2() - linerect.y1());

	if (side == 2)
	{
		vec.x = -vec.x;
		vec.y = -vec.y;
	}

	int x = vec.y;
	int y = vec.x;
	x = -x;

	wxPoint side_p(mid.x - x, mid.y - y);

	//printf("side %d\n", side);

	float min_dist = -1;
	int nearest_line = -1;
	wxPoint nearest_midpoint;

	for (DWORD l = 0; l < NumLines(); l++)
	{
		if (l != line)
		{
			rect_t line_r = l_getrect(l);
			int x1 = line_r.x1();
			int x2 = line_r.x2();
			int y1 = line_r.y1();
			int y2 = line_r.y2();

			wxPoint r1 = mid;
			wxPoint r2 = side_p;

			float u_ray = (float(x2 - x1) * float(r1.y - y1) - float(y2 - y1) * float(r1.x - x1)) /
				(float(y2 - y1) * float(r2.x - r1.x) - float(x2 - x1) * float(r2.y - r1.y));

			float u_line = (float(r2.x - r1.x) * float(r1.y - y1) - float(r2.y - r1.y) * float(r1.x - x1)) /
				(float(y2 - y1) * float(r2.x - r1.x) - float(x2 - x1) * float(r2.y - r1.y));


			if ((u_ray >= 0) &&/* (u_ray < 1) && */(u_line >= 0) && (u_line <= 1))
			{
				if (u_ray < min_dist || min_dist == -1)
				{
					nearest_line = l;
					min_dist = u_ray;
					nearest_midpoint = midpoint(line_r.tl, line_r.br);
				}

				if (u_ray == min_dist)
				{
					double nearest_mid_dist = distance_to_line(linerect.x1(), linerect.y1(), linerect.x2(), linerect.y2(),
						nearest_midpoint.x, nearest_midpoint.y);

					wxPoint this_midpoint(midpoint(line_r.tl, line_r.br));
					double this_mid_dist = distance_to_line(linerect.x1(), linerect.y1(), linerect.x2(), linerect.y2(),
						this_midpoint.x, this_midpoint.y);

					if (this_mid_dist < nearest_mid_dist)
					{
						nearest_line = l;
						min_dist = u_ray;
						nearest_midpoint = midpoint(line_r.tl, line_r.br);
					}
				}
			}
		}
	}

	if (nearest_line == -1)
	{
		//printf("No intersection\n");
		return -1;
	}
	else
	{
		if (determine_line_side(nearest_line, mid.x, mid.y))
			return FrontSecNo(nearest_line);
		else
			return BackSecNo(nearest_line);
	}
}

bool CLevel::l_needsuptex(int l, int side)
{
	CSector * sector1 = GetSector(FrontSecNo(l));
	CSector * sector2 = GetSector(BackSecNo(l));

	// False if not two-sided
	if (sector1 == NULL || sector2 == NULL) return false;

	if (side == 1)
	{
		return (sector1->ceilh > sector2->ceilh);
	}

	if (side == 2)
	{
		return (sector2->ceilh > sector1->ceilh);
	}

	return false;
}

bool CLevel::l_needsmidtex(int l, int side)
{
	if (side == 1)
	{
		if (BackSecNo(l) == -1)
			return true;
	}
	return false;
}

bool CLevel::l_needslotex(int l, int side)
{
	CSector * sector1 = GetSector(FrontSecNo(l));
	CSector * sector2 = GetSector(BackSecNo(l));

	// False if not two-sided
	if (sector1 == NULL || sector2 == NULL) return false;

	if (side == 1)
	{
		return (sector1->floorh < sector2->floorh);
	}

	if (side == 2)
	{
		return (sector2->floorh < sector1->floorh);
	}

	return false;
}

void CLevel::l_setdeftextures(int l)
{
	CLine * ln = GetLine(l);

	if (FrontSecNo(l) != -1)
	{
		strncpy(ln->sides[0].texUpper,"-", 8);
		strncpy(ln->sides[0].texNormal,"-", 8);
		strncpy(ln->sides[0].texLower,"-", 8);

		if (l_needsuptex(l, 1)) strncpy(ln->sides[0].texUpper,DEF_WALLTEX, 8);
		if (l_needsmidtex(l, 1)) strncpy(ln->sides[0].texNormal,DEF_WALLTEX, 8);
		if (l_needslotex(l, 1)) strncpy(ln->sides[0].texLower,DEF_WALLTEX, 8);
	}

	if (BackSecNo(l) != -1)
	{
		strncpy(ln->sides[1].texUpper,"-", 8);
		strncpy(ln->sides[1].texNormal,"-", 8);
		strncpy(ln->sides[1].texLower,"-", 8);

		if (l_needsuptex(l, 2)) strncpy(ln->sides[1].texUpper,DEF_WALLTEX, 8);
		if (l_needsmidtex(l, 2)) strncpy(ln->sides[1].texNormal,DEF_WALLTEX, 8);
		if (l_needslotex(l, 2)) strncpy(ln->sides[1].texLower,DEF_WALLTEX, 8);
	}
}




// lines_clockwise: Checks if a group of lines are all facing eachother
// ----------------------------------------------------------------- >>
bool CLevel::lines_clockwise(numlist_t *lines)
{
	for (DWORD a = 0; a < lines->n_numbers; a++)
	{
		int l = lines->numbers[a];

		rect_t linerect = l_getrect(l);
		wxPoint mid = midpoint(linerect.tl, linerect.br);
		wxPoint vec(linerect.x2() - linerect.x1(), linerect.y2() - linerect.y1());
		int x = vec.y;
		int y = vec.x;
		x = -x;
		wxPoint side(mid.x + x, mid.y + y);

		bool intersect = false;

		for (DWORD b = 0; b < lines->n_numbers; b++)
		{
			if (b != a)
			{
				rect_t line = l_getrect(lines->numbers[b]);

				int x1 = line.x1();
				int x2 = line.x2();
				int y1 = line.y1();
				int y2 = line.y2();

				wxPoint r1 = mid;
				wxPoint r2 = side;

				float u_ray = (float(x2 - x1) * float(r1.y - y1) - float(y2 - y1) * float(r1.x - x1)) /
					(float(y2 - y1) * float(r2.x - r1.x) - float(x2 - x1) * float(r2.y - r1.y));

				float u_line = (float(r2.x - r1.x) * float(r1.y - y1) - float(r2.y - r1.y) * float(r1.x - x1)) /
					(float(y2 - y1) * float(r2.x - r1.x) - float(x2 - x1) * float(r2.y - r1.y));

				if((u_ray >= 0)/* && (u_ray <= 1024)*/ && (u_line >= 0) && (u_line <= 1))
					intersect = true;
			}
		}

		if (!intersect)
			return true;
	}

	return false;
}

// check_cplit: Checks if line drawing ended in a 'real' sector split
// --------------------------------------------------------------- >>
bool CLevel::check_split(int split_sector)
{
	// Cycle through lines starting with the end vertex, and if we get back to the end
	// vertex before the start vertex, it means we don't have a proper sector split
	numlist_t checked_lines, a_lines, prev_verts;
	int vertex = v_getvertatpoint(ldraw_points.get_last());
	int end_vert = v_getvertatpoint(ldraw_points.points[0]);
	int start = vertex;
	prev_verts.add(start, false);
	bool done = false;

	while (!done)
	{
		// Get any lines attached to the current vertex
		a_lines.clear();
		v_getattachedlines(vertex, &a_lines);

		for (DWORD l = 0; l < a_lines.n_numbers; l++)
		{
			CLine * ln = GetLine(a_lines.numbers[l]);

			if (!checked_lines.exists(a_lines.numbers[l]) &&
				!ldraw_lines.exists(a_lines.numbers[l]))
			{
				// Ignore lines with both sides referencing our sector
				if (FrontSecNo(ln) == split_sector && BackSecNo(ln) == split_sector)
				{
					checked_lines.add(a_lines.numbers[l], true);
					continue;
				}

				if (ln->line.Start == vertex &&	FrontSecNo(ln) == split_sector)
				{
					vertex = ln->line.End;
					checked_lines.add(a_lines.numbers[l], true);
					prev_verts.add(vertex, false);

					if (vertex == end_vert) return true;

					// If we're back at the start vertex, go to the previous vertex
					if (vertex == start)
					{
						vertex = prev_verts.get_last();
						prev_verts.remove_last();

						if (prev_verts.n_numbers == 0) return false;
					}

					break;
				}

				if (ln->line.End == vertex && BackSecNo(ln) == split_sector)
				{
					vertex = ln->line.Start;
					checked_lines.add(a_lines.numbers[l], true);
					prev_verts.add(vertex, false);

					if (vertex == end_vert)	return true;

					if (vertex == start)
					{
						vertex = prev_verts.get_last();
						prev_verts.remove_last();

						if (prev_verts.n_numbers == 0) return false;
					}

					break;
				}
			}

			// If we're at the end of the attached lines without
			// coming across any not already checked or part of the sector,
			// either go back a vertex (if we're not at the start), or exit
			// the loop (and return false)
			if (l == a_lines.n_numbers - 1)
			{
				if (vertex == start) done = true;
				else
				{
					vertex = prev_verts.get_last();
					prev_verts.remove_last();

					if (prev_verts.n_numbers == 0) return false;
				}
			}
		}
	}

	return false;
}

void check_lines_twosided()
{

}

// end_linedraw: Finishes a line drawing session, builds lines/vertices/sectors
// ------------------------------------------------------------------------- >>
void CLevel::CancelLineDraw()
{
	if (m_Mode==modeLineDraw && ldraw_points.n_points>2)
	{
		m_Mode=modeLineDefs;
		OnModeLinedefs();
		MakeBackup("Line Drawing", true, true, true, false);

		TArray<CLine*>	new_lines;

		bool split = false;

		// If the last 2 points are the same, we have a sector split
		if (ldraw_points.get_last() == ldraw_points.points[ldraw_points.n_points - 2])
			split = true;

		// Add any vertices that have been 'drawn over'
		bool verts_added = false;
		for (int a = 0; a < (int)ldraw_points.n_points - 1; a++)
		{
			TArray<int> verts;
			rect_t line(ldraw_points.points[a], ldraw_points.points[a+1]);

			for (int v = 0; v < NumVertices(); v++)
			{
				CVertex * vt = GetVertex(v);

				if (int(vt->X()) == ldraw_points.points[a].x && int(vt->Y()) == ldraw_points.points[a].y)
					continue;

				if (int(vt->X()) == ldraw_points.points[a+1].x && int(vt->Y()) == ldraw_points.points[a+1].y)
					continue;

				if (distance_to_line(line.x1(), line.y1(), line.x2(), line.y2(), vt->X(), vt->Y()) < 1)
				{
					verts.Push(v);
					verts_added = true;
				}
			}

			if (verts.Size() == 0)
				continue;

			// Sort vertices by distance from the first point
			if (verts.Size() > 1)
			{
				bool done = false;

				while (!done)
				{
					done = true;
					for (int b = 0; b < verts.Size() - 1; b++)
					{
						CVertex * v1 = GetVertex(verts[b]);
						CVertex * v2 = GetVertex(verts[b+1]);

						int d1 = distance(ldraw_points.points[a].x, ldraw_points.points[a].y, v1->X(), v1->Y());
						int d2 = distance(ldraw_points.points[a].x, ldraw_points.points[a].y, v2->X(), v2->Y());

						if (d1 > d2)
						{
							int temp = verts[b];
							verts[b] = verts[b+1];
							verts[b+1] = temp;
							done = false;
						}
					}
				}
			}

			// For easy inserting convert ldraw_points to a vector
			TArray<wxPoint> points;
			for (int b = 0; b < ldraw_points.n_points; b++)
				points.Push(ldraw_points.points[b]);

			// Add points at vertices
			for (int b = 0; b < verts.Size(); b++)
				points.Insert(a + b + 1, v_getpoint(verts[b]));

			// Rebuild ldraw_points
			ldraw_points.clear();
			for (int b = 0; b < points.Size(); b++)
				ldraw_points.add(points[b], false);
		}


		// *** SPLIT SECTOR ***
		// I'm sure theres a much nicer way to do this, but the method I use actually works,
		// So it'll do for now :P
		if (split)
		{
			numlist_t a_lines; // List of lines attached to vertex being examined

			// Determine sector to split, if any.
			// Do so by finding the nearest sector to the midpoint of the last line drawn
			wxPoint mid = midpoint(ldraw_points.points[0], ldraw_points.points[1]);
			int split_sector = determine_sector(mid.x, mid.y);

			// Add a vertex at each point, also add vertex numbers to the created vertices list
			// Also, split any lines if need be, and ignore duplicate points
			for (int p = 0; p < ldraw_points.n_points; p++)
			{
				if (ldraw_points.points[p] != ldraw_points.points[p - 1])
					ldraw_verts.add(InsertVertex(ldraw_points.points[p].x, ldraw_points.points[p].y), true);

				int split_line = check_vertex_split(NumVertices() - 1);

				if (split_line != -1)
					l_split(split_line, NumVertices() - 1);
			}

			// Add lines from the vertices, also adding line numbers to the created lines list
			for (int v = 1; v < ldraw_verts.n_numbers; v++)
				ldraw_lines.add(add_line(ldraw_verts.numbers[v - 1], ldraw_verts.numbers[v]), true);

			// Merge vertices (in the entire map, for the moment)
			merge_verts();
			Deleteunusedvertices();

			// Rebuild vertices list
			ldraw_verts.clear();
			for (DWORD l = 0; l < new_lines.Size(); l++)
			{
				ldraw_verts.add(StVtNo(ldraw_lines.numbers[l]), true);
			}

			ldraw_verts.add(EnVtNo(ldraw_lines.get_last()), true);

			// If the split line(s) has/have a loose end
			if (!v_isattached_sector(v_getvertatpoint(ldraw_points.points[0]))
				|| !v_isattached_sector(v_getvertatpoint(ldraw_points.get_last())))
			{
				// Run through newly created lines, set both their sides to the 'split' sector
				for (DWORD l = 0; l < ldraw_lines.n_numbers; l++)
				{
					l_setsector(ldraw_lines.numbers[l], 1, split_sector);
					l_setsector(ldraw_lines.numbers[l], 2, split_sector);
				}
			}
			else if (!check_split(split_sector))
			{
				//printf("check_split failed\n");

				// Run through newly created lines, set both their sides to the 'split' sector
				for (DWORD l = 0; l < ldraw_lines.n_numbers; l++)
				{
					l_setsector(ldraw_lines.numbers[l], 1, split_sector);
					l_setsector(ldraw_lines.numbers[l], 2, split_sector);
				}
			}
			else
			{
				// Create a new sector
				int new_sector = InsertSector(NULL);

				// Copy split sector attributes to new sector
				if (split_sector != -1)
					map.CopySector(new_sector, GetSector(split_sector));

				// Run through newly created lines, set their first side to the new
				// sector, and their second side to the split sector
				numlist_t changed_lines;

				for (DWORD l = 0; l < ldraw_lines.n_numbers; l++)
				{
					l_setsector(ldraw_lines.numbers[l], 1, new_sector);
					l_setsector(ldraw_lines.numbers[l], 2, split_sector);
					changed_lines.add(ldraw_lines.numbers[l], true);
				}

				// Starting at the last new vertex, cycle through lines that are part of
				// the sector to be split, and set whatever side's sector reference
				// to the new sector. End once we arrive at the first created vertex. Phew.
				bool done = false;
				bool not_split = false;
				int vertex = v_getvertatpoint(ldraw_points.get_last());
				int end_vert = v_getvertatpoint(ldraw_points.points[0]);
				int start_vert = vertex;

				while (!done)
				{
					// Get any lines attached to the current vertex
					v_getattachedlines(vertex, &a_lines);

					for (DWORD l = 0; l < a_lines.n_numbers; l++)
					{
						CLine * ln = GetLine(a_lines.numbers[l]);

						if (!ldraw_lines.exists(a_lines.numbers[l]) &&
							!changed_lines.exists(a_lines.numbers[l]))
						{
							// If the line has both sides pointing to the sector to be split, skip it
							// We'll deal with these lines later
							if (FrontSecNo(ln) == split_sector && BackSecNo(ln) == split_sector)
								l++;

							if (ln->line.Start == vertex &&	FrontSecNo(ln) == split_sector)
							{
								l_setsector(a_lines.numbers[l], 1, new_sector);
								vertex = ln->line.End;
								changed_lines.add(a_lines.numbers[l], true);

								if (vertex == end_vert)	done = true;
								break;
							}

							if (ln->line.End == vertex &&
								BackSecNo(a_lines.numbers[l]) == split_sector)
							{
								l_setsector(a_lines.numbers[l], 2, new_sector);
								vertex = ln->line.Start;
								changed_lines.add(a_lines.numbers[l], true);

								if (vertex == end_vert)
									done = true;

								break;
							}
						}

						// To prevent an infinite loop
						if (l == a_lines.n_numbers - 1)
							done = true;
					}
				}

				// Reverse cycle (just to add some changed_lines for later)
				done = false;
				vertex = v_getvertatpoint(ldraw_points.get_last());
				start_vert = vertex;

				while (!done)
				{
					// Get any lines attached to the current vertex
					v_getattachedlines(vertex, &a_lines);

					for (DWORD l = 0; l < a_lines.n_numbers; l++)
					{
						CLine * ln = GetLine(a_lines.numbers[l]);

						if (!ldraw_lines.exists(a_lines.numbers[l]) &&
							!changed_lines.exists(a_lines.numbers[l]))
						{
							// If the line has both sides pointing to the sector to be split, skip it
							// We'll deal with these lines later
							if (FrontSecNo(ln) == split_sector && BackSecNo(ln) == split_sector)
								l++;

							if (ln->line.End == vertex && FrontSecNo(ln) == split_sector
								&& !changed_lines.exists(a_lines.numbers[l]))
							{
								vertex = ln->line.Start;
								changed_lines.add(a_lines.numbers[l], true);

								if (vertex == end_vert) done = true;

								break;
							}

							if (ln->line.Start == vertex &&
								BackSecNo(a_lines.numbers[l]) == split_sector
								&& !changed_lines.exists(a_lines.numbers[l]))
							{
								vertex = ln->line.End;
								changed_lines.add(a_lines.numbers[l], true);

								if (vertex == end_vert)
									done = true;

								break;
							}
						}

						// To prevent an infinite loop
						if (l == a_lines.n_numbers - 1)
							done = true;
					}
				}

				if (not_split)
				{
					for (DWORD l = 0; l < ldraw_lines.n_numbers; l++)
					{
						l_setsector(ldraw_lines.numbers[l], 1, split_sector);
						l_setsector(ldraw_lines.numbers[l], 2, split_sector);
					}
				}
				else
				{
					// Now we go through all the lines that are part of the sector to be split
					// and determine which side of the split they are on
					for (DWORD l = 0; l < NumLines(); l++)
					{
						if ((FrontSecNo(l) == split_sector || BackSecNo(l) == split_sector)
							&& !changed_lines.exists(l) && !ldraw_lines.exists(l))
						{
							int closest_line = -1;
							int min_dist = -1;

							rect_t line_rect = l_getrect(l);
							float mid_x = (float)midpoint(line_rect.tl, line_rect.br).x;
							float mid_y = (float)midpoint(line_rect.tl, line_rect.br).y;

							if (FrontSecNo(l) == split_sector)
							{
								if (line_rect.x1() > line_rect.x2())
									mid_y += 0.5f;
								if (line_rect.x2() > line_rect.x1())
									mid_y -= 0.5f;
								if (line_rect.y1() > line_rect.y2())
									mid_x -= 0.5f;
								if (line_rect.y2() > line_rect.y1())
									mid_x += 0.5f;
							}

							if (BackSecNo(l) == split_sector)
							{
								if (line_rect.x1() > line_rect.x2())
									mid_y -= 0.5f;
								if (line_rect.x2() > line_rect.x1())
									mid_y += 0.5f;
								if (line_rect.y1() > line_rect.y2())
									mid_x += 0.5f;
								if (line_rect.y2() > line_rect.y1())
									mid_x -= 0.5f;
							}

							wxPoint mid(midpoint(l_getrect(l).tl, l_getrect(l).br));

							for (DWORD a = 0; a < changed_lines.n_numbers; a++)
							{
								rect_t line2(l_getrect(changed_lines.numbers[a]).tl, l_getrect(changed_lines.numbers[a]).br);
								int dist = distance_to_line(line2.x1(), line2.y1(), line2.x2(), line2.y2(), mid.x, mid.y);

								if (min_dist == -1 || dist < min_dist)
								{
									min_dist = dist;
									closest_line = changed_lines.numbers[a];
								}
							}

							// Now figure out what sector the line should be in
							int sector = -1;

							if (FrontSecNo(l) == split_sector)
							{
								if (determine_line_side(closest_line, mid_x, mid_y))
								{
									if (FrontSecNo(closest_line) == new_sector)
										l_setsector(l, 1, new_sector);
								}
								else
								{
									if (BackSecNo(closest_line) == new_sector)
										l_setsector(l, 1, new_sector);
								}
							}

							if (BackSecNo(l) == split_sector)
							{
								if (determine_line_side(closest_line, mid_x, mid_y))
								{
									if (FrontSecNo(closest_line) == new_sector)
										l_setsector(l, 2, new_sector);
								}
								else
								{
									if (BackSecNo(closest_line) == new_sector)
										l_setsector(l, 2, new_sector);
								}
							}
						}
					}
				}

				// Check for any new non-twosided lines that have the 2s flag
				for (DWORD l = 0; l < ldraw_lines.n_numbers; l++)
				{
					CLine * ln = GetLine(ldraw_lines.numbers[l]);
					/*
					if ((ln->line.flags & LINE_TWOSIDED) && ln->sides[1].sector == -1)
					{
						ln->line.flags &= ~LINE_TWOSIDED;
					}
					*/
				}
			}
		}
		else // *** CLOSED SECTOR ADDED ***
		{
			bool new_sector_used = false;
			bool new_sector_copied = false;
			int new_sector = InsertSector(NULL);
			numlist_t new_lines;
			//numlist_t changed_sides;
			numlist_t check_sectors;
			numlist_t changed_lines;

			// Go through all new points
			for (DWORD p = 0; p < ldraw_points.n_points - 1; p++)
			{
				bool done = false;
				wxPoint p1 = ldraw_points.points[p];
				wxPoint p2 = ldraw_points.points[p + 1];
				wxPoint p3;
				int v1 = -1;
				int v2 = -1;

				if (p == 0)
					p3 = ldraw_points.points[p + 2];
				else
					p3 = ldraw_points.points[p - 1];

				// If we have a point on a line that isn't a current vertex,
				// add the vertex and split the line
				if (v_checkspot(p1.x, p1.y))
				{
					v1 = InsertVertex(p1.x, p1.y);

					if (check_vertex_split(p1) != -1)
						l_split(check_vertex_split(p1), v1);
				}
				else
					v1 = v_getvertatpoint(p1);

				if (v_checkspot(p2.x, p2.y))
				{
					v2 = InsertVertex(p2.x, p2.y);

					if (check_vertex_split(p2) != -1)
						l_split(check_vertex_split(p2), v2);
				}
				else
					v2 = v_getvertatpoint(p2);

				// If the two points we're looking at are both current vertices
				if (v1 != -1 && v2 != -1)
				{
					for (DWORD l = 0; l < NumLines(); l++)
					{
						CLine * ln = GetLine(l);
						// If the points are the front side of the line
						if (ln->line.Start == v1 && ln->line.End == v2)
						{
							// Set first side to the new sector
							if (FrontSecNo(ln) != -1 && !new_sector_copied)
							{
								map.CopySector(new_sector, FrontSec(ln));
								new_sector_copied = true;
							}

							check_sectors.add(FrontSecNo(l), true);
							l_setsector(l, 1, new_sector);

							changed_lines.add(l, true);
							done = true;
							new_sector_used = true;
							break;
						}

						// If the points are the back side of the line
						if (ln->line.End == v1 && ln->line.Start == v2)
						{
							// Set second side to the new sector
							if (FrontSecNo(ln) != -1 && !new_sector_copied)
							{
								map.CopySector(new_sector, FrontSec(l));
								new_sector_copied = true;
							}

							check_sectors.add(FrontSecNo(l), true);
							l_setsector(l, 2, new_sector);

							changed_lines.add(l, true);
							done = true;
							new_sector_used = true;
							break;
						}
					}
				}

				if (!done)
				{
					// If not, create the needed vertices and a new line
					if (v1 == -1)
						v1 = InsertVertex(p1.x, p1.y);

					if (v2 == -1)
						v2 = InsertVertex(p2.x, p2.y);

					int new_line = add_line(v1, v2);
					new_lines.add(new_line, true);
				}
			}

			// If the lines are clockwise, set their first side to the new sector
			// otherwise set their second sides to the new sector. Also set the opposite
			// side to whatever sector it's in
			numlist_t all_new_lines;
			all_new_lines.add_numlist(&new_lines, true);
			all_new_lines.add_numlist(&changed_lines, true);

			if (lines_clockwise(&all_new_lines))
			{
				for (DWORD a = 0; a < new_lines.n_numbers; a++)
				{
					int l = new_lines.numbers[a];
					int sector2 = get_side_sector(l, 2);
					check_sectors.add(sector2, true);

					if (!new_sector_copied && sector2 != -1)
					{
						map.CopySector(new_sector, GetSector(sector2));
						new_sector_copied = true;
					}

					l_setsector(l, 1, new_sector);
					l_setsector(l, 2, sector2);
					l_setdeftextures(l);
					new_sector_used = true;
				}
			}
			else
			{
				// If we have no changed lines, it means we have a column
				if (changed_lines.n_numbers == 0)
				{
					//printf("column\n");

					for (DWORD a = 0; a < new_lines.n_numbers; a++)
					{
						int l = new_lines.numbers[a];
						int sector1 = get_side_sector(l, 1);
						check_sectors.add(sector1, true);

						l_setsector(l, 1, sector1);
						l_setdeftextures(l);
						new_sector_used = false;
					}
				}
				else
				{
					for (DWORD a = 0; a < new_lines.n_numbers; a++)
					{
						int l = new_lines.numbers[a];
						int sector2 = get_side_sector(l, 2);
						check_sectors.add(sector2, true);

						if (!new_sector_copied && sector2 != -1)
						{
							map.CopySector(new_sector, GetSector(sector2));
							new_sector_copied = true;
						}

						l_setsector(l, 1, new_sector);
						l_setsector(l, 2, sector2);
						l_setdeftextures(l);
						new_sector_used = true;
					}
				}
			}

			// Now we process any lines that might need sector references changed
			// in a similar fashion to above (determine what side is in what sector)
			if (check_sectors.exists(-1))
				check_sectors.remove(-1);

			for (DWORD s = 0; s < check_sectors.n_numbers; s++)
			{
				int sector = check_sectors.numbers[s];

				for (DWORD l = 0; l < NumLines(); l++)
				{
					if (!all_new_lines.exists(l))
					{
						if (FrontSecNo(l) == sector)
						{
							int d_sector = get_side_sector(l, 1);

							if (d_sector != sector && d_sector != -1)
								l_setsector(l, 1, d_sector);
						}

						if (BackSecNo(l) == sector)
						{
							int d_sector = get_side_sector(l, 2);

							if (d_sector != sector && d_sector != -1)
								l_setsector(l, 2, d_sector);
						}
					}
				}
			}

			// Check for any new lines that need to be split
			if (verts_added)
			{
				for (int a = 0; a < new_lines.n_numbers; a++)
				{
					rect_t rect = l_getrect(new_lines.numbers[a]);

					for (int v = 0; v < NumVertices(); v++)
					{
						int l = check_vertex_split(v);
						if (new_lines.exists(l) && l != -1)
						{
							l_split(l, v);
							UncheckAll();
							m_Mode=modeVertexes;
							
							/*
							// FIXME: The vertex splitter check doesn't work yet!
							hilight_item = v;
							add_move_items();
							clear_move_items();
							hilight_item = -1;
							*/
						}
					}
				}
			}
		}
		ldraw_verts.clear();
		ldraw_lines.clear();


		// Finish up
		for(int i=NumLines()-1;i>=0;i--)
		{
			if (StVt(i)->X() == EnVt(i)->X() && StVt(i)->Y() == EnVt(i)->Y())
			{
				DeleteOneLineDef(i);
			}
		}
		Deleteunusedvertices();

		m_DrawWindow->Refresh();
		m_changed=true;
		m_NeedNodeBuild=true;
	}
	ldraw_points.clear();
}




bool CLevel::LineDrawNotAllowed()
{
	return m_Mode==modeLineDraw;
}

void CLevel::AbortLineDraw()
{
	ldraw_points.clear();
	OnModeLinedefs();
}

void CLevel::DeleteLineDrawVertex()
{
	if (ldraw_points.n_points>0)
	{
		ldraw_points.n_points--;
		m_DrawWindow->Refresh();
	}
}

void CLevel::DrawLineDraw(wxDC & DC,CRectFloat * rcClip)
{
	int i;
	int mm = DC.GetLogicalFunction();
	DC.SetLogicalFunction(wxXOR);
	for(i=0;i<(int)ldraw_points.n_points-1;i++)
	{
		MapLine(DC, hpYe, ldraw_points[i].x, ldraw_points[i].y, ldraw_points[i+1].x, ldraw_points[i+1].y);
	}
	if (lastpointvalid)
	{
		MapLine(DC, hpYe, ldraw_points[i].x, ldraw_points[i].y, lastpoint.x, lastpoint.y);
	}
	DC.SetLogicalFunction(mm);
}

void CLevel::OnMMoveLineDraw(wxDC & DC, wxRealPoint pt)
{
	if (ldraw_points.n_points>0)
	{
		const wxPoint * last = &ldraw_points[ldraw_points.n_points-1];
		int mm = DC.GetLogicalFunction();
		DC.SetLogicalFunction(wxXOR);
		if (lastpointvalid) MapLine(DC, hpYe, last->x, last->y, lastpoint.x, lastpoint.y);
		lastpoint=wxPoint((int)(pt.x+0.5), (int)(pt.y+0.5));
		if (m_SnapToGrid) SnapToGrid(&lastpoint.x, &lastpoint.y);
		lastpointvalid=true;
		MapLine(DC, hpYe, last->x, last->y, lastpoint.x, lastpoint.y);
		DC.SetLogicalFunction(mm);
	}
}

void CLevel::OnLButtonLineDraw(wxDC & DC, wxRealPoint pt)
{
	OnMMoveLineDraw(DC, pt);
	wxPoint p((int)(pt.x+0.5), (int)(pt.y+0.5));
	if (m_SnapToGrid) SnapToGrid(&p.x, &p.y);
	ldraw_points.add( p, false);
	lastpointvalid=false;
}

void CLevel::OnLinedrawStartAtVertex(wxCommandEvent & event)
{
	/// Add this vertex
	if (m_Mode==modeVertexes)
	{
		CVertex * v=GetVertex(m_Selection);
		wxPoint pt(int(v->X()), int(v->Y())); 
		lastpointvalid=false;
		ldraw_points.add(pt, false);
		OnModeLinedefs();
		m_Mode=modeLineDraw;
	}
}

void CLevel::OnToolsStartlinedrawing(wxCommandEvent & event)
{
	lastpointvalid=false;
	OnModeLinedefs();
	m_Mode=modeLineDraw;
}

void CLevel::OnUpdateNoLinedraw(wxUpdateUIEvent & event)
{
	event.Enable(m_Mode!=modeLineDraw);
}
