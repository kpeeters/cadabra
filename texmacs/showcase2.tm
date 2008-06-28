<TeXmacs|1.0.6>

<style|generic>

<\body>
  <with|prog-language|cadabra|prog-session|default|<\session>
    <\output>
      <with|font-family|rm|<with|font-size|1.41|Cadabra 0.8> (built on Thu
      Jul 20 11:17:01 CEST 2006)<next-line>Copyright (c) 2001-2006 Kasper
      Peeters \<less\>kasper.peeters@aei.mpg.de\<gtr\><next-line>Available
      under the terms of the GNU General Public License.<next-line>>Default
      startup file ~/.cadabra not present.

      \;
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      {m,n,p,q,r}::Integer(0..3).

      \\bar{#}::DiracBar.\ 

      \\Gamma{#}::GammaMatrix.

      \\psi_{m}::Spinor(dimension=4).\ 

      \\psi_{m}::GammaTraceless.\ 

      \\delta_{m n}::KroneckerDelta.\ 

      { \\delta_{m n}, A_{m}, \\psi_{m}, \\Gamma_{#} }::SortOrder.
    </input>

    <\output>
      Assigning property Integer to m, n, p, q, r.

      Assigning property DiracBar to \\bar.

      Assigning property GammaMatrix to \\Gamma.

      Assigning property Spinor to \\psi.

      Assigning property GammaTraceless to \\psi.

      Assigning property KroneckerDelta to \\delta.

      Assigning property SortOrder to \\delta, A, \\psi, \\Gamma.

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      ::PostDefaultRules{ @@prodsort!(%), @@eliminate_kr!(%) }.
    </input>

    <\output>
      Assigning property PostDefaultRules to .

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      \\bar{\\psi_{m}} (\\Gamma_{m n} + A_{m} \\Gamma_{n}) \\Gamma_{p q r}
      \\psi_{r};
    </input>

    <\output>
      1:= <with|mode|math|<wide|\<psi\><rsub|m>|\<bar\>><space|0.25spc>(\<Gamma\><rsub|m*n>+A<rsub|m><space|0.25spc>\<Gamma\><rsub|n>)<space|0.25spc>\<Gamma\><rsub|p*q*r><space|0.25spc>\<psi\><rsub|r>;>

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      @distribute!(%);
    </input>

    <\output>
      1:= <with|mode|math|<wide|\<psi\><rsub|m>|\<bar\>><space|0.25spc>\<Gamma\><rsub|m*n><space|0.25spc>\<Gamma\><rsub|p*q*r><space|0.25spc>\<psi\><rsub|r>+A<rsub|m><space|0.25spc><wide|\<psi\><rsub|m>|\<bar\>><space|0.25spc>\<Gamma\><rsub|n><space|0.25spc>\<Gamma\><rsub|p*q*r><space|0.25spc>\<psi\><rsub|r>;>

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      @join!(%);
    </input>

    <\output>
      1:= <with|mode|math|<wide|\<psi\><rsub|m>|\<bar\>><space|0.25spc>(6<space|0.25spc>\<delta\><rsub|n*p><space|0.25spc>\<Gamma\><rsub|m*q*r>+6<space|0.25spc>\<delta\><rsub|m*q><space|0.25spc>\<delta\><rsub|n*p><space|0.25spc>\<Gamma\><rsub|r>)<space|0.25spc>\<psi\><rsub|r>+A<rsub|m><space|0.25spc><wide|\<psi\><rsub|m>|\<bar\>><space|0.25spc>(\<Gamma\><rsub|n*p*q*r>+3<space|0.25spc>\<delta\><rsub|n*p><space|0.25spc>\<Gamma\><rsub|q*r>)<space|0.25spc>\<psi\><rsub|r>;>

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      @distribute!(%);
    </input>

    <\output>
      1:= <with|mode|math|6<space|0.25spc>\<delta\><rsub|n*p><space|0.25spc><wide|\<psi\><rsub|m>|\<bar\>><space|0.25spc>\<Gamma\><rsub|m*q*r><space|0.25spc>\<psi\><rsub|r>+6<space|0.25spc>\<delta\><rsub|m*q><space|0.25spc>\<delta\><rsub|n*p><space|0.25spc><wide|\<psi\><rsub|m>|\<bar\>><space|0.25spc>\<Gamma\><rsub|r><space|0.25spc>\<psi\><rsub|r>+A<rsub|m><space|0.25spc><wide|\<psi\><rsub|m>|\<bar\>><space|0.25spc>\<Gamma\><rsub|n*p*q*r><space|0.25spc>\<psi\><rsub|r>+3<space|0.25spc>\<delta\><rsub|n*p><space|0.25spc>A<rsub|m><space|0.25spc><wide|\<psi\><rsub|m>|\<bar\>><space|0.25spc>\<Gamma\><rsub|q*r><space|0.25spc>\<psi\><rsub|r>;>

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      @remove_gamma_trace!(%);
    </input>

    <\output>
      1:= <with|mode|math|-6<space|0.25spc>\<delta\><rsub|m*q><space|0.25spc>\<delta\><rsub|n*p><space|0.25spc><wide|\<psi\><rsub|m>|\<bar\>><space|0.25spc>\<Gamma\><rsub|r><space|0.25spc>\<psi\><rsub|r>+6<space|0.25spc>\<delta\><rsub|n*p><space|0.25spc><wide|\<psi\><rsub|m>|\<bar\>><space|0.25spc>\<Gamma\><rsub|q><space|0.25spc>\<psi\><rsub|m>-A<rsub|m><space|0.25spc><wide|\<psi\><rsub|m>|\<bar\>><space|0.25spc>\<Gamma\><rsub|n*p><space|0.25spc>\<psi\><rsub|q>+A<rsub|m><space|0.25spc><wide|\<psi\><rsub|m>|\<bar\>><space|0.25spc>\<Gamma\><rsub|n*q><space|0.25spc>\<psi\><rsub|p>-A<rsub|m><space|0.25spc><wide|\<psi\><rsub|m>|\<bar\>><space|0.25spc>\<Gamma\><rsub|p*q><space|0.25spc>\<psi\><rsub|n>-3<space|0.25spc>\<delta\><rsub|n*p><space|0.25spc>A<rsub|m><space|0.25spc><wide|\<psi\><rsub|m>|\<bar\>><space|0.25spc>\<psi\><rsub|q>;>

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      @remove_gamma_trace!(%);
    </input>

    <\output>
      1:= <with|mode|math|6<space|0.25spc>\<delta\><rsub|n*p><space|0.25spc><wide|\<psi\><rsub|m>|\<bar\>><space|0.25spc>\<Gamma\><rsub|q><space|0.25spc>\<psi\><rsub|m>-A<rsub|m><space|0.25spc><wide|\<psi\><rsub|m>|\<bar\>><space|0.25spc>\<Gamma\><rsub|n*p><space|0.25spc>\<psi\><rsub|q>+A<rsub|m><space|0.25spc><wide|\<psi\><rsub|m>|\<bar\>><space|0.25spc>\<Gamma\><rsub|n*q><space|0.25spc>\<psi\><rsub|p>-A<rsub|m><space|0.25spc><wide|\<psi\><rsub|m>|\<bar\>><space|0.25spc>\<Gamma\><rsub|p*q><space|0.25spc>\<psi\><rsub|n>-3<space|0.25spc>\<delta\><rsub|n*p><space|0.25spc>A<rsub|m><space|0.25spc><wide|\<psi\><rsub|m>|\<bar\>><space|0.25spc>\<psi\><rsub|q>;>

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      \;
    </input>
  </session>>
</body>

<\initial>
  <\collection>
    <associate|font-base-size|8>
  </collection>
</initial>