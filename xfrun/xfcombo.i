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
#include <xfce4-modules/constants.h>
#include <xfce4-modules/combo.h>



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
    gchar *xdg_dir=xfce_resource_save_location (XFCE_RESOURCE_CACHE,"/",TRUE);
    gchar *f=g_build_filename(xdg_dir,RUN_DBH_FILE,NULL); 

    g_free(xdg_dir);

    if (access(f,F_OK)!=0) return;
    
    XFC_read_history(combo_info,f);
    XFC_set_combo(combo_info,NULL);
    g_free(f);
    extra_key_completion((gpointer)(combo_info->entry));
    return;
}


void unload_xfc(void){
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
    gchar *module;
    gchar *librarydir=g_build_filename (LIBDIR, "xfce4", "modules",NULL);
   
    if (xfc_fun) return xfc_fun;
    module = g_module_build_path(librarydir, "xfce4_combo");
    g_free(librarydir);
    
    xfc_cm=g_module_open (module, 0);
    if (!xfc_cm) {
	g_warning("cannot load xfce4-module %s\n",module);
	return NULL;
    }
    
    if (!g_module_symbol (xfc_cm, "module_init",(gpointer) &(module_init)) ) {
	g_error("g_module_symbol(module_init) != FALSE\n");
        exit(1);
    }

    
    xfc_fun = (*module_init)();
 
    g_free(module);
    return xfc_fun;
}


/* XXX: these two functions could be module-loaded from xffm... */
static void  save_flags(gchar *in_cmd, gboolean interm, gboolean hold){
    DBHashTable *runflags;
    GString *gs;
    int *flags;
    gchar *xdg_dir=xfce_resource_save_location (XFCE_RESOURCE_CACHE,"/",TRUE);
    
    gchar *g=g_build_filename(xdg_dir,RUN_FLAG_FILE,NULL);
    g_free(xdg_dir);
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
    gchar *xdg_dir=xfce_resource_save_location (XFCE_RESOURCE_CACHE,"/",TRUE);
    gchar *g=g_build_filename(xdg_dir,RUN_FLAG_FILE,NULL);

    g_free(xdg_dir);
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


