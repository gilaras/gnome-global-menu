#include <gtk/gtk.h>

#include <config.h>

#include "quirks.h"
#include "menubar.h"

#define LOG(fmt, args...) g_message("<GnomenuQuirk>::" fmt, ## args)
#define LOG_FUNC_NAME LOG("%s", __func__)

static GQueue * default_quirks = NULL;
typedef struct {
	gchar * match;
	GnomenuQuirkMask mask;
} QuirkEntry;

static GFlagsValue * _get_quirk_value_by_nick(gchar * nick){
	static GTypeClass * type_class = NULL;
		if(type_class == NULL) type_class = g_type_class_ref(gnomenu_quirk_mask_get_type());
	return g_flags_get_value_by_nick(type_class, nick);
}
static void _add_default_quirks_from_string(gchar * string){
	gchar ** lines;
	gchar ** words;
	gchar * word;
	gchar ** quirks;
	int i, j, k, l;
	QuirkEntry * entry ;
	lines = g_strsplit(string, "\n", 0);
	if(lines)
	for(i = 0; lines[i]; i++){
		words = g_strsplit(lines[i], ":", 0);
		if(!words) continue;
		l = g_strv_length(words);
		if(l !=2 ){
			g_warning("Irregular conf file line(%d):\n%s", l, lines[i]);
			g_strfreev(words);
			continue;
		}
		word = g_strstrip(words[0]);
		if(!word || word[0] == '#'){
			g_strfreev(words);
			continue;
		}
		entry = g_new0(QuirkEntry, 1);
		entry->match = g_strdup(word);
		word = g_strstrip(words[1]);
		entry->mask = GNOMENU_QUIRK_NONE;
		quirks = g_strsplit(word, ",", 0);
		if(quirks){
			l = g_strv_length(quirks);
			for(k = 0; k < l; k++){
				word = g_strstrip(quirks[k]);
				GFlagsValue * value = _get_quirk_value_by_nick(word);
				if(value){
					entry->mask |= value->value;
					LOG("found quirk for %s: %s=%d", entry->match, word, value->value);
				} else {
					g_warning("unknown quirk type: %s", word);
				}
			}
			g_strfreev(quirks);
		}
		g_queue_push_tail(default_quirks, entry);	
	}
	g_strfreev(lines);
}
static void _add_default_quirks_from_file(gchar * file){
	gchar * contents;
	gint length;
	GError * error = NULL;
	if(g_file_get_contents(file, &contents, &length, &error)) {
		LOG("file opened");
		_add_default_quirks_from_string(contents);
	}else{
		LOG("%s", error->message);
	}
	g_free(contents);
}
static void _load_default_quirks(){
	if(!default_quirks) {
		gchar * file;
		default_quirks = g_queue_new();
		_add_default_quirks_from_file(SYSCONFDIR G_DIR_SEPARATOR_S "libgnomenu.conf");
		file = g_strconcat(g_get_home_dir(), G_DIR_SEPARATOR_S ".libgnomenu.conf", NULL);
		_add_default_quirks_from_file(file);
	}
}
struct quirk_match_data {
	gchar * prgname;
	GnomenuQuirkMask rt;
};
static void _match_quirk(QuirkEntry * entry, struct quirk_match_data * data){
	LOG("match %s against %s", data->prgname, entry->match);
	if(g_pattern_match_simple(entry->match, data->prgname)) {
		data->rt = entry->mask;
	}
}
/**
 * gnomenu_get_default_quirk:
 *
 * Returns: the default quirk for current application
 */
GnomenuQuirkMask gnomenu_get_default_quirk(){
	_load_default_quirks();
	QuirkEntry * entry;
	static GnomenuQuirkMask default_quirk = GNOMENU_QUIRK_NONE;
	static gboolean loaded = FALSE;
	if(!loaded){
		struct quirk_match_data data = { g_get_prgname(), GNOMENU_QUIRK_NONE};
		g_queue_foreach(default_quirks, _match_quirk, &data);
		default_quirk = data.rt;
		LOG("default_quirk = %d", default_quirk);
		loaded = TRUE;
	}	
	return default_quirk;	
}
GType gnomenu_menu_bar_type = 0;
void gtk_module_init(int * argc, char **argv[]){
/*initialize */
	GnomenuQuirkMask mask = gnomenu_get_default_quirk();
	gnomenu_menu_bar_type = gnomenu_menu_bar_get_type();
	if(GNOMENU_HAS_QUIRK(mask, IGNORE)){
		gnomenu_menu_bar_type = 0; /*disable gtk_menu_bar_get_type hack*/
	}
}
