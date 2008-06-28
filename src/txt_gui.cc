
/*




  Description:
  - Two input channels: user input and kernel output. 
  - The user input should be grabbed using readline and sent to the kernel.
  - Take TeX-like input with labels attached to symbols (as described in the docs)
  from the kernel, and display it as eqascii does. Allow for selections to be made
  in this expression.



 */



	if(interactive_) {
		if(txt.substr(0,4)=="\\esc") {
			interactive_=false;
			tree<str_node> *pr=expressions.back();
			out_->sender("\n\n\n\n\n\n\n\n\033[8A\033[s");
			print_subtree(*pr, pr->begin(), pr->end(), false);
			print_prompt();
			}
		else if(txt.substr(0,1)==" ") {
			mark.ranges.push_back(treemarker::range(cursor_begin, cursor_end));
			redraw_interactive_();
			}
		else if(txt.substr(0,6)=="\\right") {
			if(cursor_end.is_valid()) {
				++cursor_begin;
				++cursor_end;
				redraw_interactive_();
				}
			}
		else if(txt.substr(0,5)=="\\left") {
			if(cursor_begin.node->prev_sibling) {
				--cursor_begin;
				--cursor_end;
				redraw_interactive_();
				}
			}
		else if(txt.substr(0,9)=="\\pagedown") {
			if(cursor_end.is_valid()) {
				++cursor_end;
				redraw_interactive_();
				}
			}
		else if(txt.substr(0,7)=="\\pageup") {
			sibling_iterator tmp=cursor_end;
			--tmp;
			if(tmp!=cursor_begin) {
				--cursor_end;
				redraw_interactive_();
				}
			}
		else if(txt.substr(0,5)=="\\down") {
			if(cursor_begin.node->first_child!=0) {
				cursor_begin=cursor_begin.node->first_child;
				cursor_end=cursor_begin;
				++cursor_end;
				redraw_interactive_();
				}
			}
		else if(txt.substr(0,3)=="\\up") {
			if(cursor_begin.node->parent->data.name!="\\head") {
				cursor_begin=cursor_begin.node->parent;
				cursor_end=cursor_begin;
				++cursor_end;
				redraw_interactive_();
				}
			}
		else {
			iterator it=settings.begin();
			bool done=false; // FIXME: once selection is in submodules, this can go away
			while((it=find(it, settings.end(), str_node("\\action")))!=settings.end()) {
				if(settings.child(it,0).name==txt.substr(0,1)) {
					done=true;
					cout << "ACTION! " << settings.child(it,1).name << endl;
					}
				++it;
				}
			if(!done)
				mark.ranges.clear();
			redraw_interactive_();
			}
		}
