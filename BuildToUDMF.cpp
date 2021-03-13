// BuildToUDMF.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"
#include "files.h"
#include "buildtypes.h"
#include "tarray.h"
#include <map>

struct VertexLink
{
	int x, y;
	TArray<int> touchingLines;
};

TArray<VertexLink> realvertices;


void CollectVertices()
{
	TArray<VertexLink> vertexes;
	VertexLink vo;

	for (int i = 0; i < numwalls; i++)
	{
		// First collect all positons.
		bool insertedfirst = false, insertedsecond = false;
		int ii = wall[i].nextwall;
		for (unsigned j = 0; j < vertexes.Size(); j++)
		{
			if (!insertedfirst && vertexes[j].x == wall[i].x && vertexes[j].y == wall[i].y)
			{
				vertexes[j].touchingLines.Push(i);
				insertedfirst = true;
			}
			if (!insertedsecond && vertexes[j].x == wall[ii].x && vertexes[j].y == wall[ii].y)
			{
				vertexes[j].touchingLines.Push(i);
				insertedsecond = true;
			}
		}
		if (!insertedfirst)
		{
			vo.x = wall[i].x;
			vo.y = wall[i].y;
			vo.touchingLines.Push(i);
			vertexes.Push(vo);
		}
		if (!insertedsecond)
		{
			vo.x = wall[ii].x;
			vo.y = wall[ii].y;
			vo.touchingLines.Push(i);
			vertexes.Push(vo);
		}
	}
#if 0
	for (auto& v : vertexes)
	{
		while (v.touchingLines.Size() > 0)
		{
			TArray<int> group;
			bool changedsomething = true;
			group.Push(v.touchingLines[0]);
			v.touchingLines.Delete(0);
			while (changedsomething)
			{
				changedsomething = false;
				for (unsigned j = 0; j < v.touchingLines.Size(); j++)
				{
					int thisindex = v.touchingLines[j];
					auto thiswall = &wall[thisindex];
					for (unsigned g = 0; g < group.Size(); g++)
					{
						auto thatindex = group[g];
						auto thatwall = &wall[thatindex];
						if (thiswall->nextwall == thatindex || thatwall->nextwall == thisindex || thiswall->point2 == thatindex || thatwall->point2 == thisindex)
						{
							group.Push(thisindex);
							v.touchingLines.Delete(j);
							j--;
							changedsomething = true;
							break;
						}
					}
				}
			}
			VertexLink vnew;
			vnew.x = v.x;
			vnew.y = v.y;
			vnew.touchingLines = std::move(group);
			realvertices.Push(vnew);
		}
	}
#else
	realvertices = std::move(vertexes);
#endif
}



void ConvertBuildMap(FileReader& fr, FileWriter* fw)
{
	spritetype startpos;
	engineLoadBoard(fr, startpos);
	CollectVertices();

#if 0
	fw->Write("PWAD\3\0\0\0\xc\0\0\0", 12);
	fw->Write("\0\0\0\0\0\0\0\0MAP01\0\0\0", 16);
	fw->Write("\0\0\0\0\0\0\0\0TEXTMAP\0", 16);
	fw->Write("\0\0\0\0\0\0\0\0ENDMAP\0\0", 16);
	int pos = fw->Tell();
#endif

	fw->Printf("namespace = \"zdoom\";\n");
	for (int i = 0; i < numsprites; i++)
	{
		fw->Printf("thing // %d\n{\n", i);
		fw->Printf("x = %f;\n", -sprite[i].x / 16.);
		fw->Printf("y = %f;\n", sprite[i].y / 16.);
		fw->Printf("z = %f;\n", -sprite[i].z / 256.);
		fw->Printf("skill1 = true;\nsingle = true;\n");	// fixme
		fw->Printf("cstat = %d;\n", sprite[i].cstat);
		fw->Printf("type = %d;\n", sprite[i].picnum);
		fw->Printf("shade = %d;\n", sprite[i].shade);
		fw->Printf("pal = %d;\n", sprite[i].pal);
		fw->Printf("clipdist = %d;\n", sprite[i].clipdist);
		fw->Printf("blend = %d;\n", sprite[i].blend);
		fw->Printf("xrepeat = %d;\n", sprite[i].xrepeat);
		fw->Printf("yrepeat = %d;\n", sprite[i].yrepeat);
		fw->Printf("xoffset = %d;\n", sprite[i].xoffset);
		fw->Printf("yoffset = %d;\n", sprite[i].yoffset);
		fw->Printf("sectnum = %d;\n", sprite[i].sectnum);
		fw->Printf("statnum = %d;\n", sprite[i].statnum);
		fw->Printf("angle = %d;\n", sprite[i].ang * 360/2048);
		fw->Printf("owner = %d;\n", sprite[i].owner);
		fw->Printf("xvel = %d;\n", sprite[i].xvel);
		fw->Printf("yvel = %d;\n", sprite[i].yvel);
		fw->Printf("zvel = %d;\n", sprite[i].zvel);
		fw->Printf("zvel = %d;\n", sprite[i].zvel);
		fw->Printf("id = %d;\n", sprite[i].lotag);
		fw->Printf("special = %d;\n", sprite[i].hitag);
		fw->Printf("extra = %d;\n", sprite[i].extra);
		fw->Printf("detail = %d;\n", sprite[i].detail);

		// only to allow viewing the values
		fw->Printf("arg0 = %d;\n", sprite[i].sectnum);
		fw->Printf("arg1 = %d;\n", sprite[i].statnum);
		fw->Printf("arg2 = %d;\n", sprite[i].extra);
		fw->Printf("}\n");
	}

	// Inject a player start.
	fw->Printf("thing // %d\n{\n", -1);
	fw->Printf("x = %f;\n", -startpos.x / 16.);
	fw->Printf("y = %f;\n", startpos.y / 16.);
	fw->Printf("z = %f;\n", -startpos.z / 256.);
	fw->Printf("skill1 = true;\nsingle = true;\n");	// fixme
	fw->Printf("skill2 = true;\n");	// fixme
	fw->Printf("skill3 = true;\n");	// fixme
	fw->Printf("skill4 = true;\n");	// fixme
	fw->Printf("skill5 = true;\n");	// fixme
	fw->Printf("type = %d;\n", 1);
	fw->Printf("sectnum = %d;\n", startpos.sectnum);
	fw->Printf("angle = %d;\n", startpos.ang * 360 / 2048);
	fw->Printf("}\n");


	for (unsigned i = 0; i < realvertices.Size(); i++)
	{
		fw->Printf("vertex // %d\n{\n", i);
		fw->Printf("x = %f;\n", -realvertices[i].x / 16.);
		fw->Printf("y = %f;\n", realvertices[i].y / 16.);
		fw->Printf("}\n");
	}
	for (int i = 0; i < numwalls; i++)
	{
		if (wall[i].nextwall < 0 || wall[i].nextwall > i)
		{
			fw->Printf("linedef // %d\n{\n", i);
			int ii = wall[i].point2;
			int vi = realvertices.FindEx([=](const VertexLink& v) { return wall[i].x == v.x && wall[i].y == v.y; });
			int vii = realvertices.FindEx([=](const VertexLink& v) { return wall[ii].x == v.x && wall[ii].y == v.y; });
			fw->Printf("v1 = %d;\n", vi);
			fw->Printf("v2 = %d;\n", vii);
			fw->Printf("sidefront = %d;\n", i);
			if (wall[i].nextwall >= 0)
			{
				fw->Printf("sideback = %d;\n", wall[i].nextwall);
				fw->Printf("twosided = true;\n");
			}
			fw->Printf("}\n");
		}
	}
	for (int i = 0; i < numwalls; i++)
	{
		int sectornum = -1;
		for (int j = 0; j < numsectors; j++)
		{
			if (sector[j].wallptr <= i && sector[j].wallnum + sector[j].wallptr > i)
			{
				sectornum = j;
				break;
			}
		}
		fw->Printf("sidedef // %d\n{\n", i);
		fw->Printf("sector = %d;\n", sectornum);
		fw->Printf("cstat = %d;\n", wall[i].cstat);
		fw->Printf("texturemiddle = \"#%05d\";\n", wall[i].picnum);
		fw->Printf("texturetop = \"#%05d\";\n", wall[i].overpicnum);
		fw->Printf("shade = %d;\n", wall[i].shade);
		fw->Printf("pal = %d;\n", wall[i].pal);
		fw->Printf("scalex = %d;\n", wall[i].xrepeat);
		fw->Printf("scaley = %d;\n", wall[i].yrepeat);
		fw->Printf("offsetx = %d;\n", wall[i].xpan);
		fw->Printf("offsety = %d;\n", wall[i].ypan);
		fw->Printf("lotag = %d;\n", wall[i].lotag);
		fw->Printf("hitag = %d;\n", wall[i].hitag);
		fw->Printf("extra = %d;\n", wall[i].extra);
		fw->Printf("}\n");
	}

	for (int i = 0; i < numsectors; i++)
	{
		fw->Printf("sector // %d\n{\n", i);
		fw->Printf("heightceiling = %d;\n", -sector[i].ceilingz / 256);
		fw->Printf("heightfloor = %d;\n", -sector[i].floorz / 256);
		fw->Printf("floorstat = %d;\n", sector[i].floorstat);
		fw->Printf("ceilingstat = %d;\n", sector[i].ceilingstat);
		fw->Printf("floorstat = %d;\n", sector[i].floorstat);
		fw->Printf("textureceiling = \"#%05d\";\n", sector[i].ceilingpicnum);
		fw->Printf("texturefloor = \"#%05d\";\n", sector[i].floorpicnum);
		fw->Printf("ceilingshade = %d;\n", sector[i].ceilingshade);
		fw->Printf("floorshade = %d;\n", sector[i].floorshade);
		fw->Printf("ceilingpal = %d;\n", sector[i].ceilingpal);
		fw->Printf("floorpal = %d;\n", sector[i].floorpal);
		fw->Printf("xpanningceiling = %d;\n", sector[i].ceilingxpan);
		fw->Printf("xpanningfloor = %d;\n", sector[i].floorxpan);
		fw->Printf("ypanningceiling = %d;\n", sector[i].ceilingypan);
		fw->Printf("ypanningfloor = %d;\n", sector[i].floorypan);
		fw->Printf("lotag = %d;\n", sector[i].lotag);
		fw->Printf("hitag = %d;\n", sector[i].hitag);
		fw->Printf("extra = %d;\n", sector[i].extra);
		fw->Printf("ceilingheinum = %d;\n", sector[i].ceilingheinum);
		fw->Printf("floorheinum = %d;\n", sector[i].floorheinum);
		fw->Printf("visibility = %d;\n", sector[i].visibility);
		fw->Printf("fogpal = %d;\n", sector[i].fogpal);
		fw->Printf("}\n");
	}
#if 0
	int size = fw->Tell() - pos;
	fw->Seek(28, SEEK_SET);
	fw->Write(&pos, 4);
	fw->Write(&size, 4);
	fw->Close();
#endif
}


/*
int main(int argc, const char** argv)
{
	if (argc < 3)
	{
		printf("Usage: BuildToUDMF src.map dst.wad\n");
		return 3;
	}
	FileReader fr;
	if (!fr.OpenFile(argv[1]))
	{
		printf("%s: file not found\n", argv[1]);
		return 3;
	}
	FileWriter *fw = FileWriter::Open(argv[2]);
	if (!fw)
	{
		printf("%s: unable to open for writing\n", argv[2]);
		return 3;
	}
	ConvertBuildMap(fr, fw);
}

*/