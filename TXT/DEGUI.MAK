# this is a modified degui makefile (1.25 beta)
# modified by Scott "Jerry" Lawrence  jerry@absynth.com
# for the Turaco Arcade Games Editor
#
# use this makefile if you plan on building Turaco using Degui
# === It is recommended that you use Degui! ===
#


CC = gcc
AR = ar

OPTIONS = \
	-O2\
	-W\
	-Wall\
	-fomit-frame-pointer\
	-s

OBJS = \
	alert.o\
	color.o\
	d_box.o\
	d_butt1.o\
	d_butt2.o\
	d_check1.o\
	d_check2.o\
	d_clear.o\
	d_ctext.o\
	d_edit1.o\
	d_edit2.o\
	d_hlist1.o\
	d_list1.o\
	d_list2.o\
	d_panra.o\
	d_pansu.o\
	d_panri.o\
	d_pangr.o\
	d_progr1.o\
	d_progr2.o\
	d_radio1.o\
	d_radio2.o\
	d_sbox.o\
	d_scroll.o\
	d_slidr1.o\
	d_slidr2.o\
	d_text.o\
	file_sel.o\
	gfx_sel.o\
	internal.o\
	menu.o\

# these were taken out so that degui can be built properly on my 486's
CPPOBJS = \
	bitmap_o.o\
	box_o.o\
	button_o.o\
	checkb_o.o\
	clear_o.o\
	color_sc.o\
	ctext_o.o\
	degui.o\
	dialog.o\
	dialog_d.o\
	dialog_o.o\
	editt_o.o\
	grid_o.o\
	group_o.o\
	hlist_o.o\
	inifile.o\
	keybrd_o.o\
	list_o.o\
	menu_o.o\
	panra.o\
	pansu.o\
	panri.o\
	pangr.o\
	proc_o.o\
	progre_o.o\
	radio_o.o\
	sbox_o.o\
	scroll_o.o\
	slider_o.o\
	tbox_o.o\
	text_o.o\
	wbutt_o.o\
	window_o.o

INCLUDE_DIRS = ../include
INCLUDES = $(addprefix -I,$(INCLUDE_DIRS))

all : libdegui.a

libdegui.a : $(OBJS)
	$(AR) rs $@ $^
	move libdegui.a ..\\lib

%o : %cc
	$(CC) -c $(INCLUDES) $(OPTIONS) $<

%o : %c
	$(CC) -c $(INCLUDES) $(OPTIONS) $<

clean:
	del *.exe
	del *.o
	del *.gdt
