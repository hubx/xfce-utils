/*  
 *  Copyright (C) 2003 Edscott Wilson Garcia <edscott@users.sourceforge.net>
 *
 *  This code is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __xfcombo_i__
#define __xfcombo_i__

#include <gmodule.h>
#include <dbh.h>

typedef struct _xfc_combo_info_t xfc_combo_info_t;

#define CURRENT_RUNFLAG "xffm.runflag.2.dbh"
#define CURRENT_RUN_HISTORY "xffm.runlist.2.dbh"
#define RUN_FLAG_FILE g_get_home_dir(),"/.xfce4/xffm/",CURRENT_RUNFLAG
#define RUN_DBH_FILE g_get_home_dir(),"/.xfce4/xffm/",CURRENT_RUN_HISTORY

struct _xfc_combo_info_t{
    GtkCombo 	*combo;  
    GtkEntry 	*entry;  
    gchar 	*active_dbh_file; 
    gpointer	cancel_user_data; 
    gpointer	activate_user_data;
    void	(*cancel_func)(GtkEntry *entry,gpointer cancel_user_data);
    void	(*activate_func)(GtkEntry *entry,gpointer activate_user_data);
    GList	*list;
    GList	*limited_list;
    GList	*old_list;
};

typedef struct _xfc_combo_functions xfc_combo_functions;
struct _xfc_combo_functions {
 /* exported: */
    gboolean	(*xfc_is_in_history)(char *path2dbh_file,char *path2find);
    gboolean	(*xfc_set_combo)(xfc_combo_info_t *combo_info, char *token);
    void	(*xfc_set_blank)(xfc_combo_info_t *combo_info);
    void	(*xfc_set_entry)(xfc_combo_info_t *combo_info,char *entry_string);
    void	(*xfc_save_to_history)(char *path2dbh_file,char *path2save);
    void	(*xfc_remove_from_history)(char *path2dbh_file,char *path2remove);
    void	(*xfc_read_history)(xfc_combo_info_t *combo_info, gchar *path2dbh_file);
    void 	(*xfc_clear_history)(xfc_combo_info_t *combo_info);
    xfc_combo_info_t *(*xfc_init_combo)(GtkCombo *combo);
    xfc_combo_info_t *(*xfc_destroy_combo)(xfc_combo_info_t *combo_info);
 /* imported (or null): */
    int		(*extra_key_completion)(gpointer extra_key_data);
    gpointer	extra_key_data;
};

#define XFC_is_in_history (*(load_xfc()->xfc_is_in_history))
#define XFC_set_combo (*(load_xfc()->xfc_set_combo))
#define XFC_set_blank (*(load_xfc()->xfc_set_blank))
#define XFC_set_entry (*(load_xfc()->xfc_set_entry))
#define XFC_save_to_history (*(load_xfc()->xfc_save_to_history))
#define XFC_remove_from_history (*(load_xfc()->xfc_remove_from_history))
#define XFC_read_history (*(load_xfc()->xfc_read_history))
#define XFC_clear_history (*(load_xfc()->xfc_clear_history))
#define XFC_init_combo (*(load_xfc()->xfc_init_combo))
#define XFC_destroy_combo (*(load_xfc()->xfc_destroy_combo))

xfc_combo_functions *load_xfc(void);
static void  save_flags(gchar *in_cmd, gboolean interm, gboolean hold);
static void  recover_flags(gchar *in_cmd,gboolean *interm,gboolean *hold);

G_MODULE_IMPORT
int extra_key_completion(gpointer extra_key_data);

static xfc_combo_info_t *combo_info=NULL;
static xfc_combo_functions *xfc_fun=NULL;
static GModule *xfc_cm=NULL;

int extra_key_completion(gpointer user_data)
{
    	GtkEntry *entry=(GtkEntry *)user_data; 
	gboolean interm,hold;
      	gchar *choice = g_strdup((gchar *)gtk_entry_get_text(entry));
	recover_flags(choice, &interm, &hold);
        gtk_toggle_button_set_active ((GtkToggleButton *)checkbox,interm);	
#ifdef DEBUG
	printf("at extra_key_completion!\n");
#endif
	g_free(choice);
	return FALSE;
}

static void set_run_combo(xfc_combo_info_t *combo_info)
{
    char *p;
    gchar *f=g_strconcat(RUN_DBH_FILE,NULL); 
    gboolean interm;

    if (access(f,F_OK)!=0) return;
    
    XFC_read_history(combo_info,f);
    XFC_set_combo(combo_info,NULL);
    g_free(f);
    extra_key_completion((gpointer)(combo_info->entry));
    return;
}


void unload_xfc(void){
	g_free(xfc_fun);
	xfc_fun = NULL;
	if (!g_module_close(xfc_cm)){
	   g_warning("g_module_close(xfc_cm) != TRUE\n");
	}
#ifdef DEBUG
	else {
           g_message ("module libxfce4_combo unloaded");
	}	   
#endif
	xfc_cm=NULL;
}

xfc_combo_functions *load_xfc(void){
    xfc_combo_functions *(*module_init)(void) ;
    gchar *library, *module;
   
    if (xfc_fun) return xfc_fun;
    
    library=g_strconcat("libxfce4_combo.",G_MODULE_SUFFIX, NULL);
    module = g_build_filename (LIBDIR, "xfce4", "modules",library, NULL);
    
    xfc_cm=g_module_open (module, 0);
    if (!xfc_cm) {
	g_warning("cannot load xfce4-module %s\n",module);
	return NULL;
    }
    
    if (!g_module_symbol (xfc_cm, "module_init",(gpointer *) &(module_init)) ) {
	g_error("g_module_symbol(module_init) != FALSE\n");
        exit(1);
    }

    
    xfc_fun = (*module_init)();
 
    if (
       !g_module_symbol (xfc_cm, "xfc_is_in_history",(gpointer *) &(xfc_fun->xfc_is_in_history))  ||
       !g_module_symbol (xfc_cm, "xfc_set_combo", (gpointer *) &(xfc_fun->xfc_set_combo)) ||
       !g_module_symbol (xfc_cm, "xfc_set_blank", (gpointer *) &(xfc_fun->xfc_set_blank)) ||
       !g_module_symbol (xfc_cm, "xfc_set_entry", (gpointer *) &(xfc_fun->xfc_set_entry)) ||
       !g_module_symbol (xfc_cm, "xfc_save_to_history", (gpointer *) &(xfc_fun->xfc_save_to_history)) ||
       !g_module_symbol (xfc_cm, "xfc_remove_from_history", (gpointer *) &(xfc_fun->xfc_remove_from_history)) ||
       !g_module_symbol (xfc_cm, "xfc_read_history", (gpointer *) &(xfc_fun->xfc_read_history)) ||
       !g_module_symbol (xfc_cm, "xfc_clear_history", (gpointer *) &(xfc_fun->xfc_clear_history)) ||
       !g_module_symbol (xfc_cm, "xfc_init_combo", (gpointer *) &(xfc_fun->xfc_init_combo)) ||
       !g_module_symbol (xfc_cm, "xfc_destroy_combo", (gpointer *) &(xfc_fun->xfc_destroy_combo)) ||
       !g_module_symbol (xfc_cm, "xfc_init_combo", (gpointer *) &(xfc_fun->xfc_init_combo)) ||
       !g_module_symbol (xfc_cm, "xfc_destroy_combo", (gpointer *) &(xfc_fun->xfc_destroy_combo)) 
   	     ) {
	/*g_error("g_module_symbol() != FALSE\n");*/
        /*exit(1);*/
    	g_free(library);
    	g_free(module);
	return NULL;
    }
#ifdef DEBUG
    g_message ("module %s successfully loaded", library);	    
#endif
    g_free(library);
    g_free(module);
    return xfc_fun;
}


/* XXX: these two functions should be module-loaded from xffm... */
static void  save_flags(gchar *in_cmd, gboolean interm, gboolean hold){
    DBHashTable *runflags;
    GString *gs;
    int *flags;
    gchar *g=g_strconcat(RUN_FLAG_FILE,NULL);
    if((runflags = DBH_open(g)) == NULL)
    {
	if((runflags = DBH_create(g, 11)) == NULL){
	    g_warning("Cannot create %s\n",g);
	    return;
	}
    }
    gs = g_string_new(in_cmd);
    sprintf((char *)DBH_KEY(runflags), "%10u", g_string_hash(gs));
    g_string_free(gs, TRUE);
    flags = (int *)runflags->data;
    flags[0]=interm;
    flags[1]=hold;
    DBH_set_recordsize(runflags, 2*sizeof(int));
    
    DBH_update(runflags);
    DBH_close(runflags);
#ifdef DEBUG
    printf("flags saved in dbh file for %s\n",in_cmd);
#endif
}

static void  recover_flags(gchar *in_cmd,gboolean *interm,gboolean *hold){
    DBHashTable *runflags;
    GString *gs;
    int *flags;
    gchar *g=g_strconcat(RUN_FLAG_FILE,NULL);
    if((runflags = DBH_open(g)) == NULL)
    {
#ifdef DEBUG
	    g_warning("Cannot open %s\n",g);
#endif
	    *interm=0;
	    *hold=0;
	    return;
    }
    gs = g_string_new(in_cmd);
    sprintf((char *)DBH_KEY(runflags), "%10u", g_string_hash(gs));
    g_string_free(gs, TRUE);
    flags = (int *)runflags->data;
    DBH_load(runflags);
    *interm = flags[0];
    *hold = flags[1];
    DBH_close(runflags);
#ifdef DEBUG
    printf("flags recovered from dbh file for %s, interm=%d hold=%d\n",in_cmd,*interm,*hold);
#endif
}
#endif


