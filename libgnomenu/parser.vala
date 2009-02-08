using Gtk;

namespace Gnomenu {
	/**
	 * Parser converts xml to widgets.
	 *
	 * A sub-parser is created if a sumne is encountered.
	 */
	public class Parser {
		public static void parse(MenuBar menubar, string description) throws GLib.Error {
			var parser = new Parser();
			var timer = new Timer();
			parser.shell = menubar;
			MarkupParseContext context = 
				new MarkupParseContext(parser.functions, 0, parser, null);
			context.parse(description, -1);
			message("Parser consumed: %lf for %ld bytes", timer.elapsed(null),
					description.size());
		}

		Parser () {
			position = 0; 
			inside_item = false;
			this.shell = null;
			MarkupParser parser_funcs = {
					start_element,
					end_element,
					null, null, null
			};
			Memory.copy(&this.functions, &parser_funcs, sizeof(MarkupParser));
		}
		MarkupParser functions;
		Shell shell;
		Item item;

		int position;
		bool inside_item;
		bool item_has_submenu;
		Parser child_parser; /*to hold the ref count*/

		private void start_element (MarkupParseContext context, 
				string element_name, 
				string[] attribute_names, 
				string[] attribute_values) throws MarkupError {
			switch(element_name){
				case "menu":
					if(inside_item == false) {
						/*Ignore it, allowing 
						 * <menu> <item/><item/><item/> </menu>
						 */
					} else {
		//				weak MenuItem item = gtk_menu_shell_get_item(shell, position) as MenuItem;
						if(item.sub_shell == null) {
							item.sub_shell = (Menu) item.get_data("_saved_menu_");
						}	
						child_parser = new Parser();
						child_parser.shell = item.sub_shell;
						g_markup_parse_context_push(context, functions, child_parser);
						item_has_submenu = true;
					}
				break;
				case "item":
					/*NOTE: after the first time we has(position) == false,
					 * it should be false forever)*/
					item = shell.get_item(position) as MenuItem;
					if(item != null) {
						setup_item(item, attribute_names, attribute_values);
					} else {
						item = new MenuItem();
						item.set_data_full("_saved_menu_", (new Menu()).ref(), g_object_unref);
						shell.append_item(item);
						item.item_position = position;
						setup_item(item, attribute_names, attribute_values);
					}
					inside_item = true;
					item_has_submenu = false;
				break;
				default:
					throw new MarkupError.UNKNOWN_ELEMENT("unkown element");
			}
		}
		private void setup_item(Item item, 
				string[] attr_names, 
				string[] attr_vals) {
			weak string label = null;
			weak string icon = null;
			weak string type = null;
			weak string state = null;
			weak string font = null;
			weak string id = null;
			weak string accel = null;
			bool sensitive = true;
			bool visible = true;
			bool underline = true;
			g_markup_collect_attributes("item", attr_names, attr_vals, null,
					GMarkupCollectType.STRING | GMarkupCollectType.OPTIONAL,
					"label", &label, 
					GMarkupCollectType.STRING | GMarkupCollectType.OPTIONAL,
					"type", &type, 
					GMarkupCollectType.STRING | GMarkupCollectType.OPTIONAL,
					"state", &state, 
					GMarkupCollectType.STRING | GMarkupCollectType.OPTIONAL,
					"font", &font, 
					GMarkupCollectType.STRING | GMarkupCollectType.OPTIONAL,
					"id", &id, 
					GMarkupCollectType.STRING | GMarkupCollectType.OPTIONAL,
					"icon", &icon, 
					GMarkupCollectType.STRING | GMarkupCollectType.OPTIONAL,
					"accel", &accel, 
					GMarkupCollectType.TRISTATE,
					"visible", &visible, 
					GMarkupCollectType.TRISTATE,
					"underline", &underline, 
					GMarkupCollectType.TRISTATE,
					"sensitive", &sensitive, 
					GMarkupCollectType.INVALID
					);
			if(visible != false)
				visible = true;
			
			if(sensitive != false)
				sensitive = true;

			if(underline != false)
				underline = true;

			item.item_id = id;
			item.item_visible = visible;
			item.item_use_underline = underline;
			item.item_sensitive = sensitive;
			item.item_type = type;
			item.item_accel_text = accel;
			item.item_label = label;
			item.item_icon= icon;
			item.item_state = state;
			item.item_font = font;
		}
		private void end_element (MarkupParseContext context, 
				string element_name) throws MarkupError {
			switch(element_name) {
				case "menu":
					if(inside_item) {
						/* stop the child parser */
						g_markup_parse_context_pop(context);
						gtk_menu_shell_truncate(child_parser.shell, child_parser.position);
						child_parser = null;
					} else {
						gtk_menu_shell_truncate(shell, position);
					}
					break;
				case "item":
					if(!item_has_submenu) {
		//				var item = gtk_menu_shell_get_item(shell, position) as MenuItem;
						item.sub_shell = null;
					}
					inside_item = false;
					position++;
				break;
			}
		}
	}
}
