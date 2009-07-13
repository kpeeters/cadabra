#!/usr/bin/env python
# render.py

# Experiments with using the mathtext expression rendering library
# on top of a cairo drawing surface.
# 
# based on
#
#  http://code.google.com/p/mathtex/
#  http://gsoc-mathtex.blogspot.com/
#
#  http://www.oluyede.org/blog/writing-a-widget-using-cairo-and-pygtk-28/
#  http://www.tortall.net/mu/wiki/PyGTKCairoTutorial

import pygtk
import gtk, gobject, cairo

from mathtex.parser import MathtexParser
from mathtex.boxmodel import ship
from mathtex.fonts import BakomaFonts
from mathtex.fonts import StixFonts
from mathtex.backends.backend_cairo import MathtexBackendCairo

class MathCell(gtk.DrawingArea):
    """Cell for display of maths."""

    __gsignals__ = { "expose-event": "override" }
  
    def __init__(self):
        gtk.DrawingArea.__init__(self)
        self.parser = MathtexParser()
        self.bakoma = BakomaFonts()
        self.stix = StixFonts()
# #          self.parser.parse(r"$R_{\mu\nu\rho}{}^{\sigma} = \nabla_\mu F_{\nu\rho}^\sigma$", 
        self.box =  \
            self.parser.parse(r"$x_{1,2}=\frac{-b \pm \sqrt{b^2 - 4ac}}{2a}$",
                            self.bakoma, 18, 99.0)
        self.rects, self.glyphs, self.bbox =  ship(0, 0, self.box)
      
    def do_expose_event(self, event):
        cr = self.window.cairo_create()
        cr.rectangle(event.area.x, event.area.y, 
                     event.area.width, event.area.height)
        cr.clip()

        width = self.bbox[2] - self.bbox[0]
        height = self.box.height
        depth = self.box.depth

        backend = MathtexBackendCairo()
        backend.set_canvas_size(self.box.width, self.box.height, 
                                self.box.depth, 100)
        backend.render(self.glyphs, self.rects)
        backend.render_to_context(cr)

 #       self.draw(cr, *self.window.get_size())

    def draw(self, cr, width, height):
        cr.set_source_rgb(0.5, 0.5, 0.7)
        cr.rectangle(0.5, 0.5, .5*width, .5*height)
        cr.fill()

def run(Widget):
    window = gtk.Window()
    window.connect("delete-event", gtk.main_quit)
    widget = MathCell()
    widget.show()
    window.add(widget)
    window.present()
    gtk.main()

if __name__ == "__main__":
    run(MathCell)
