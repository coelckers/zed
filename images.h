#ifndef __IMAGE_H
#define __IMAGE_H


extern unsigned char img_new[];
extern int size_new;
extern unsigned char img_open[];
extern int size_open;
extern unsigned char img_save[];
extern int size_save;
extern unsigned char img_cut[];
extern int size_cut;
extern unsigned char img_copy[];
extern int size_copy;
extern unsigned char img_paste[];
extern int size_paste;
extern unsigned char img_delete[];
extern int size_delete;
extern unsigned char img_acs[];
extern int size_acs;
extern unsigned char img_fs[];
extern int size_fs;
extern unsigned char img_conv[];
extern int size_conv;
extern unsigned char img_minus[];
extern int size_minus;
extern unsigned char img_plus[];
extern int size_plus;
extern unsigned char img_center[];
extern int size_center;
extern unsigned char img_goto[];
extern int size_goto;
extern unsigned char img_grid[];
extern int size_grid;
extern unsigned char img_snap[];
extern int size_snap;
extern unsigned char img_linetype[];
extern int size_linetype;
extern unsigned char img_things[];
extern int size_things;
extern unsigned char img_textures[];
extern int size_textures;
extern unsigned char img_stat[];
extern int size_stat;
extern unsigned char img_rect[];
extern int size_rect;
extern unsigned char img_poly[];
extern int size_poly;
extern unsigned char img_curve[];
extern int size_curve;
extern unsigned char img_tt[];
extern int size_tt;
extern unsigned char img_v[];
extern int size_v;
extern unsigned char img_s[];
extern int size_s;
extern unsigned char img_l[];
extern int size_l;
extern unsigned char img_map[];
extern int size_map;
extern unsigned char img_check[];
extern int size_check;
extern unsigned char img_print[];
extern int size_print;
extern unsigned char img_compile[];
extern int size_compile;


#define bitmap(n)  wxBitmap(wxImage(wxMemoryInputStream((const char*)img_##n, (size_t)size_##n), wxBITMAP_TYPE_PNG))

#endif
