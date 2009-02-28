#!/usr/bin/env python
# render.py

# Experiments with using the matplotlib.mathtext expression rendering library
# on top of a cairo drawing surface.
# 
# based on
#
#  http://www.oluyede.org/blog/writing-a-widget-using-cairo-and-pygtk-28/

import gtk

class EggClockFace(gtk.DrawingArea):
    def __init__(self):
        super(EggClockFace, self).__init__()
        self.connect("expose_event", self.expose)
        
    def expose(self, widget, event):
        context = widget.window.cairo_create()
        # set a clip region for the expose event
#        context.rectangle(event.area.x, event.area.y,
#                          event.area.width, event.area.height)
#        context.clip()
        self.draw(context)
        return False

def main():
    window = gtk.Window()
    clock = EggClockFace()
    
    window.add(clock)
    window.connect("destroy", gtk.main_quit)
    window.show_all()
    
    gtk.main()
    
if __name__ == "__main__":
    main()
