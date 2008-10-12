using GLib;
using GLibCompat;
namespace GMarkupDoc {
	public interface DocumentModel: GLib.Object {
		private static StringChunk strings = null;
		private static uint unique;
		public abstract weak Node root {get;}
		public abstract weak HashTable<weak string, weak Node> dict {get;}
		public abstract weak string name_attr {get;}
		public virtual Text CreateText(string text) {
			Text rt = new Text(this, text);
			rt.name = S("TEXT" + unique.to_string());
			unique++;
			return rt;
		}
	
		public virtual Special CreateSpecial(string text) {
			Special rt =new Special(this, text);
			rt.name =  S("SPECIAL" + unique.to_string());
			unique++;
			return rt;
		}
		public virtual Tag CreateTag(string tag) {
			Tag rt = new Tag(this, tag);
			rt.name = S("TAG" + unique.to_string());
			unique++;
			return rt;
		}
		public virtual Tag CreateTagWithAttributes(string tag, 
				string[] attr_names, string[] attr_values) {
			Tag t = CreateTag(tag);
			t.set_attributes(attr_names, attr_values);
			return t;
		}
		public virtual weak string S(string s) {
			if(strings == null) {
				strings = new StringChunk(1024);
				unique = 1;
			}
			return strings.insert_const(s);
		}
		public abstract signal void updated(Node node, string prop);
		public abstract signal void inserted(Node parent, Node node, int pos);
		public abstract signal void removed(Node parent, Node node);
		public abstract signal void activated(Node node, Quark detail);
		public virtual void activate(Node node, Quark detail) {
			this.activated(node, detail);
		}
		public virtual void transverse(Node node, TransverseFunc func) {
			Queue<weak Node> queue = new Queue<weak Node>();
			queue.push_head(node);
			while(!queue.is_empty()) {
				weak Node n = queue.pop_head();
				func(n);
				foreach(weak Node child in n.children) {
					queue.push_tail(child);
				}
			}
		}
		public delegate bool TransverseFunc(Node node);
	}

}
