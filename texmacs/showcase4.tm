<TeXmacs|1.0.6>

<style|generic>

<\body>
  <with|prog-language|cadabra|prog-session|default|<\session>
    <\output>
      <with|font-family|rm|<with|font-size|1.41|Cadabra 0.8> (built on Wed
      Jul 26 09:30:40 CEST 2006)<next-line>Copyright (c) 2001-2006 Kasper
      Peeters \<less\>kasper.peeters@aei.mpg.de\<gtr\><next-line>Available
      under the terms of the GNU General Public License.<next-line>Default
      startup file <nbsp>/.cadabra not present.>
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      { s, s' }::Coordinate.
    </input>

    <\output>
      Assigning property Coordinate to s, s'.

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      { p(s) \\theta(s) + \\partial_{s}{x(s)} \\theta(s), p(s') \\theta(s') +
      \\partial_{s'}{x(s')} \\theta(s') };
    </input>

    <\output>
      4:= <with|mode|math|{p(s)+\<theta\>(s)+\<partial\><rsub|s>x(s)<space|0.25spc>\<theta\>(s),p(s<rprime|'>)<space|0.25spc>\<theta\>(s<rprime|'>)+\<partial\><rsub|s<rprime|'>>x(s<rprime|'>)<space|0.25spc>\<theta\>(s<rprime|'>)};>

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      \;
    </input>
  </session>>
</body>