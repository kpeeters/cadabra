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
      ::KeepHistory(false).

      {m,n,p,q,r,s,t,u,v,w,a,b,c,d,e,f}::Indices(vector).

      W_{m n p q}::WeylTensor.
    </input>

    <\output>
      Assigning property KeepHistory to .

      Assigning property Indices to m, n, p, q, r, s, t, u, v, w, a, b, c, d,
      e, f.

      Assigning property WeylTensor to W.

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      R41:= W_{m n a b} W_{n p b c} W_{p s c d} W_{s m d a};

      R42:= W_{m n a b} W_{n p b c} W_{m s c d} W_{s p d a};

      R43:= W_{m n a b} W_{p s b a} W_{m n c d} W_{p s d c};

      R44:= W_{m n a b} W_{m n b a} W_{p s c d} W_{p s d c};

      R45:= W_{m n a b} W_{n p b a} W_{p s c d} W_{s m d c};

      R46:= W_{m n a b} W_{p s b a} W_{m p c d} W_{n s d c};

      R47:= W_{m n}^{m n} W_{p q}^{p q} W_{r s}^{r s} W_{t u}^{t u};\ 
    </input>

    <\output>
      R41:= <with|mode|math|W<space|0.25spc><rsub|m*n*a*b><space|0.25spc>W<space|0.25spc><rsub|n*p*b*c><space|0.25spc>W<space|0.25spc><rsub|p*s*c*d><space|0.25spc>W<space|0.25spc><rsub|s*m*d*a>;>

      R42:= <with|mode|math|W<space|0.25spc><rsub|m*n*a*b><space|0.25spc>W<space|0.25spc><rsub|n*p*b*c><space|0.25spc>W<space|0.25spc><rsub|m*s*c*d><space|0.25spc>W<space|0.25spc><rsub|s*p*d*a>;>

      R43:= <with|mode|math|W<space|0.25spc><rsub|m*n*a*b><space|0.25spc>W<space|0.25spc><rsub|p*s*b*a><space|0.25spc>W<space|0.25spc><rsub|m*n*c*d><space|0.25spc>W<space|0.25spc><rsub|p*s*d*c>;>

      R44:= <with|mode|math|W<space|0.25spc><rsub|m*n*a*b><space|0.25spc>W<space|0.25spc><rsub|m*n*b*a><space|0.25spc>W<space|0.25spc><rsub|p*s*c*d><space|0.25spc>W<space|0.25spc><rsub|p*s*d*c>;>

      R45:= <with|mode|math|W<space|0.25spc><rsub|m*n*a*b><space|0.25spc>W<space|0.25spc><rsub|n*p*b*a><space|0.25spc>W<space|0.25spc><rsub|p*s*c*d><space|0.25spc>W<space|0.25spc><rsub|s*m*d*c>;>

      R46:= <with|mode|math|W<space|0.25spc><rsub|m*n*a*b><space|0.25spc>W<space|0.25spc><rsub|p*s*b*a><space|0.25spc>W<space|0.25spc><rsub|m*p*c*d><space|0.25spc>W<space|0.25spc><rsub|n*s*d*c>;>

      R47:= <with|mode|math|W<space|0.25spc><rsub|m*n><space|0.25spc><rsup|m*n><space|0.25spc>W<space|0.25spc><rsub|p*q><space|0.25spc><rsup|p*q><space|0.25spc>W<space|0.25spc><rsub|r*s><space|0.25spc><rsup|r*s><space|0.25spc>W<space|0.25spc><rsub|t*u><space|0.25spc><rsup|t*u>;>

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      @asym!(%){^{m},^{n},^{p},^{q},^{r},^{s},^{t},^{u}}:

      @substitute!(%)( W_{a b}^{c d} -\<gtr\> W_{a b c d} ):

      @indexsort!(%):

      @collect_terms!(%):

      @canonicalise!(%):

      @collect_terms!(%);
    </input>

    <\output>
      R47:= <with|mode|math|<frac|1|840><space|0.25spc>W<space|0.25spc><rsub|m*n*p*q><space|0.25spc>W<space|0.25spc><rsub|m*n*p*q><space|0.25spc>W<space|0.25spc><rsub|r*s*t*u><space|0.25spc>W<space|0.25spc><rsub|r*s*t*u>-<frac|2|105><space|0.25spc>W<space|0.25spc><rsub|m*n*p*q><space|0.25spc>W<space|0.25spc><rsub|m*n*p*r><space|0.25spc>W<space|0.25spc><rsub|q*s*t*u><space|0.25spc>W<space|0.25spc><rsub|r*s*t*u>+<frac|1|420><space|0.25spc>W<space|0.25spc><rsub|m*n*p*q><space|0.25spc>W<space|0.25spc><rsub|m*n*r*s><space|0.25spc>W<space|0.25spc><rsub|p*q*t*u><space|0.25spc>W<space|0.25spc><rsub|r*s*t*u>-<frac|4|105><space|0.25spc>W<space|0.25spc><rsub|m*n*p*q><space|0.25spc>W<space|0.25spc><rsub|m*n*r*s><space|0.25spc>W<space|0.25spc><rsub|p*t*r*u><space|0.25spc>W<space|0.25spc><rsub|q*u*s*t>+<frac|2|105><space|0.25spc>W<space|0.25spc><rsub|m*n*p*q><space|0.25spc>W<space|0.25spc><rsub|m*r*p*s><space|0.25spc>W<space|0.25spc><rsub|n*t*q*u><space|0.25spc>W<space|0.25spc><rsub|r*t*s*u>-<frac|4|105><space|0.25spc>W<space|0.25spc><rsub|m*n*p*q><space|0.25spc>W<space|0.25spc><rsub|m*r*p*s><space|0.25spc>W<space|0.25spc><rsub|n*t*r*u><space|0.25spc>W<space|0.25spc><rsub|q*t*s*u>;>

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      basisR4:= { @(R41), @(R42), @(R43), @(R44), @(R45), @(R46), @(R47) };
    </input>

    <\output>
      basisR4:= <with|mode|math|{W<space|0.25spc><rsub|m*n*a*b><space|0.25spc>W<space|0.25spc><rsub|n*p*b*c><space|0.25spc>W<space|0.25spc><rsub|p*s*c*d><space|0.25spc>W<space|0.25spc><rsub|s*m*d*a>,W<space|0.25spc><rsub|m*n*a*b><space|0.25spc>W<space|0.25spc><rsub|n*p*b*c><space|0.25spc>W<space|0.25spc><rsub|m*s*c*d><space|0.25spc>W<space|0.25spc><rsub|s*p*d*a>,W<space|0.25spc><rsub|m*n*a*b><space|0.25spc>W<space|0.25spc><rsub|p*s*b*a><space|0.25spc>W<space|0.25spc><rsub|m*n*c*d><space|0.25spc>W<space|0.25spc><rsub|p*s*d*c>,W<space|0.25spc><rsub|m*n*a*b><space|0.25spc>W<space|0.25spc><rsub|m*n*b*a><space|0.25spc>W<space|0.25spc><rsub|p*s*c*d><space|0.25spc>W<space|0.25spc><rsub|p*s*d*c>,W<space|0.25spc><rsub|m*n*a*b><space|0.25spc>W<space|0.25spc><rsub|n*p*b*a><space|0.25spc>W<space|0.25spc><rsub|p*s*c*d><space|0.25spc>W<space|0.25spc><rsub|s*m*d*c>,W<space|0.25spc><rsub|m*n*a*b><space|0.25spc>W<space|0.25spc><rsub|p*s*b*a><space|0.25spc>W<space|0.25spc><rsub|m*p*c*d><space|0.25spc>W<space|0.25spc><rsub|n*s*d*c>,<frac|1|840><space|0.25spc>W<space|0.25spc><rsub|m*n*p*q><space|0.25spc>W<space|0.25spc><rsub|m*n*p*q><space|0.25spc>W<space|0.25spc><rsub|r*s*t*u><space|0.25spc>W<space|0.25spc><rsub|r*s*t*u>-<frac|2|105><space|0.25spc>W<space|0.25spc><rsub|m*n*p*q><space|0.25spc>W<space|0.25spc><rsub|m*n*p*r><space|0.25spc>W<space|0.25spc><rsub|q*s*t*u><space|0.25spc>W<space|0.25spc><rsub|r*s*t*u>+<frac|1|420><space|0.25spc>W<space|0.25spc><rsub|m*n*p*q><space|0.25spc>W<space|0.25spc><rsub|m*n*r*s><space|0.25spc>W<space|0.25spc><rsub|p*q*t*u><space|0.25spc>W<space|0.25spc><rsub|r*s*t*u>-<frac|4|105><space|0.25spc>W<space|0.25spc><rsub|m*n*p*q><space|0.25spc>W<space|0.25spc><rsub|m*n*r*s><space|0.25spc>W<space|0.25spc><rsub|p*t*r*u><space|0.25spc>W<space|0.25spc><rsub|q*u*s*t>+<frac|2|105><space|0.25spc>W<space|0.25spc><rsub|m*n*p*q><space|0.25spc>W<space|0.25spc><rsub|m*r*p*s><space|0.25spc>W<space|0.25spc><rsub|n*t*q*u><space|0.25spc>W<space|0.25spc><rsub|r*t*s*u>-<frac|4|105><space|0.25spc>W<space|0.25spc><rsub|m*n*p*q><space|0.25spc>W<space|0.25spc><rsub|m*r*p*s><space|0.25spc>W<space|0.25spc><rsub|n*t*r*u><space|0.25spc>W<space|0.25spc><rsub|q*t*s*u>};>

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      W_{p q r s} W_{p t r u} W_{t v q w} W_{u v s w} - W_{p q r s} W_{p q t
      u} W_{r v t w} W_{s v u w};
    </input>

    <\output>
      10:= <with|mode|math|W<space|0.25spc><rsub|p*q*r*s><space|0.25spc>W<space|0.25spc><rsub|p*t*r*u><space|0.25spc>W<space|0.25spc><rsub|t*v*q*w><space|0.25spc>W<space|0.25spc><rsub|u*v*s*w>-W<space|0.25spc><rsub|p*q*r*s><space|0.25spc>W<space|0.25spc><rsub|p*q*t*u><space|0.25spc>W<space|0.25spc><rsub|r*v*t*w><space|0.25spc>W<space|0.25spc><rsub|s*v*u*w>;>

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      @decompose!(%){ @(basisR4) }:

      @list_sum(%):

      @collect_terms!(%);
    </input>

    <\output>
      10:= <with|mode|math|{0,1,0,0,0,(<frac|-1|4>),0};>

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