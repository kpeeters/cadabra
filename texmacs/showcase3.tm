<TeXmacs|1.0.6>

<style|generic>

<\body>
  <with|prog-language|cadabra|prog-session|default|<\session>
    <\output>
      <with|font-family|rm|<with|font-size|1.41|Cadabra 0.8> (built on Thu
      Jul 20 21:30:38 CEST 2006)<next-line>Copyright (c) 2001-2006 Kasper
      Peeters \<less\>kasper.peeters@aei.mpg.de\<gtr\><next-line>Available
      under the terms of the GNU General Public License.<next-line>Default
      startup file <nbsp>/.cadabra not present.>
    </output>

    \;

    <\textput>
      In this example we want to show the invariance of the super-Maxwell
      action.

      We follow the notation on page 25 & 26 of West's book.

      \;
    </textput>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      { a,b,c,d,e }::Indices(vector).

      \\bar{#}::DiracBar.

      { \\partial{#}, \\ppartial{#} }::PartialDerivative.

      { A_{a}, f_{a b} }::Depends(\\partial, \\ppartial).

      { \\epsilon, \\gamma_{#} }::Depends(\\bar).

      \\lambda::Depends(\\bar, \\partial).

      { \\lambda, \\gamma_{#} }::NonCommuting.

      { \\lambda, \\epsilon }::Spinor(dimension=4, type=Majorana).

      { \\epsilon, \\lambda }::SortOrder.

      { \\epsilon, \\lambda }::AntiCommuting.

      \\lambda::SelfAntiCommuting.

      \\gamma_{#}::GammaMatrix.

      \\delta{#}::Accent.

      f_{a b}::AntiSymmetric.

      \\delta_{a b}::KroneckerDelta.
    </input>

    <\output>
      Assigning property Indices to a, b, c, d, e.

      Assigning property DiracBar to \\bar.

      Assigning property PartialDerivative to \\partial, \\ppartial.

      Assigning property Depends to A, f.

      Assigning property Depends to \\epsilon, \\gamma.

      Assigning property Depends to \\lambda.

      Assigning property NonCommuting to \\lambda, \\gamma.

      Assigning property Spinor to \\lambda, \\epsilon.

      Assigning property SortOrder to \\epsilon, \\lambda.

      Assigning property AntiCommuting to \\epsilon, \\lambda.

      Assigning property SelfAntiCommuting to \\lambda.

      Assigning property GammaMatrix to \\gamma.

      Assigning property Accent to \\delta.

      Assigning property AntiSymmetric to f.

      Assigning property KroneckerDelta to \\delta.

      \ 
    </output>

    <\textput>
      Useful default properties which write terms in canonical form and
      collect them:

      \;
    </textput>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      ::PostDefaultRules( @@prodsort!(%), @@prodflatten!(%),
      @@rename_dummies!(%), @@canonicalise!(%), @@collect_terms!(%) ).
    </input>

    <\output>
      Assigning property PostDefaultRules to .

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      susy:= { \\delta{A_{a}} = \\bar{\\epsilon} \\gamma_{a} \\lambda,\ 

      \ \ \ \ \ \ \ \ \ \\delta{\\lambda} = -(1/2) \\gamma_{a b} \\epsilon
      f_{a b} };
    </input>

    <\output>
      susy:= <with|mode|math|{\<delta\>A<rsub|a>=<wide|\<epsilon\>|\<bar\>><space|0.25spc>\<gamma\><rsub|a><space|0.25spc>\<lambda\>,\<delta\>\<lambda\>=(<frac|-1|2>)<space|0.25spc>\<gamma\><rsub|a*b><space|0.25spc>\<epsilon\><space|0.25spc>f<rsub|a*b>};>

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      S:= -(1/4) f_{a b} f_{a b} - (1/2) \\bar{\\lambda} \\gamma_{a}
      \\partial_{a}{\\lambda};
    </input>

    <\output>
      S:= <with|mode|math|-<frac|1|4><space|0.25spc>f<rsub|a*b><space|0.25spc>f<rsub|a*b>-<frac|1|2><space|0.25spc><wide|\<lambda\>|\<bar\>><space|0.25spc>\<gamma\><rsub|a><space|0.25spc>\<partial\><rsub|a>\<lambda\>;>

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      @vary!(%)( f_{a b} -\<gtr\> \\partial_{a}{\\delta{A_{b}}} -
      \\partial_{b}{\\delta{A_{a}}},

      \ \ \ \ \ \ \ \ \ \ \ \\lambda -\<gtr\> \\delta{\\lambda} );
    </input>

    <\output>
      S:= <with|mode|math|-<frac|1|2><space|0.25spc>((\<partial\><rsub|a>\<delta\>A<rsub|b>-\<partial\><rsub|b>\<delta\>A<rsub|a>)<space|0.25spc>f<rsub|a*b>)-<frac|1|2><space|0.25spc><wide|\<delta\>\<lambda\>|\<bar\>><space|0.25spc>\<gamma\><rsub|a><space|0.25spc>\<partial\><rsub|a>\<lambda\>-<frac|1|2><space|0.25spc><wide|\<lambda\>|\<bar\>><space|0.25spc>\<gamma\><rsub|a><space|0.25spc>\<partial\><rsub|a>\<delta\>\<lambda\>;>

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      @distribute!(%);
    </input>

    <\output>
      S:= <with|mode|math|-\<partial\><rsub|a>\<delta\>A<rsub|b><space|0.25spc>f<rsub|a*b>-<frac|1|2><space|0.25spc><wide|\<delta\>\<lambda\>|\<bar\>><space|0.25spc>\<gamma\><rsub|a><space|0.25spc>\<partial\><rsub|a>\<lambda\>-<frac|1|2><space|0.25spc><wide|\<lambda\>|\<bar\>><space|0.25spc>\<gamma\><rsub|a><space|0.25spc>\<partial\><rsub|a>\<delta\>\<lambda\>;>

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      @substitute!(%)( @(susy) ): @prodrule!(%): @distribute!(%):
      @unwrap!(%);
    </input>

    <\output>
      S:= <with|mode|math|<wide|\<epsilon\>|\<bar\>><space|0.25spc>\<gamma\><rsub|a><space|0.25spc>\<partial\><rsub|b>\<lambda\><space|0.25spc>f<rsub|a*b>+<frac|1|4><space|0.25spc><wide|\<gamma\><rsub|a*b><space|0.25spc>\<epsilon\>|\<bar\>><space|0.25spc>f<rsub|a*b><space|0.25spc>\<gamma\><rsub|c><space|0.25spc>\<partial\><rsub|c>\<lambda\>+<frac|1|4><space|0.25spc><wide|\<lambda\>|\<bar\>><space|0.25spc>\<gamma\><rsub|a><space|0.25spc>\<gamma\><rsub|b*c><space|0.25spc>\<epsilon\><space|0.25spc>\<partial\><rsub|a>f<rsub|b*c>;>

      \ 
    </output>

    <\textput>
      Rewrite the Dirac bar acting on the product.

      \;
    </textput>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      @rewrite_diracbar!(%);
    </input>

    <\output>
      Warning: assuming Minkowski signature.

      S:= <with|mode|math|<wide|\<epsilon\>|\<bar\>><space|0.25spc>\<gamma\><rsub|a><space|0.25spc>\<partial\><rsub|b>\<lambda\><space|0.25spc>f<rsub|a*b>-<frac|1|4><space|0.25spc><wide|\<epsilon\>|\<bar\>><space|0.25spc>\<gamma\><rsub|a*b><space|0.25spc>\<gamma\><rsub|c><space|0.25spc>\<partial\><rsub|c>\<lambda\><space|0.25spc>f<rsub|a*b>+<frac|1|4><space|0.25spc><wide|\<lambda\>|\<bar\>><space|0.25spc>\<gamma\><rsub|a><space|0.25spc>\<gamma\><rsub|b*c><space|0.25spc>\<epsilon\><space|0.25spc>\<partial\><rsub|a>f<rsub|b*c>;>

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      @substitute!(%)( \\partial_{c}{f_{a b}} -\<gtr\> \\ppartial_{c}{f_{a
      b}} ):

      @pintegrate!(%){\\ppartial}:\ 

      @rename!(%){\\ppartial}{\\partial}:

      @prodrule!(%): @unwrap!(%);
    </input>

    <\output>
      S:= <with|mode|math|<wide|\<epsilon\>|\<bar\>><space|0.25spc>\<gamma\><rsub|a><space|0.25spc>\<partial\><rsub|b>\<lambda\><space|0.25spc>f<rsub|a*b>-<frac|1|4><space|0.25spc><wide|\<epsilon\>|\<bar\>><space|0.25spc>\<gamma\><rsub|a*b><space|0.25spc>\<gamma\><rsub|c><space|0.25spc>\<partial\><rsub|c>\<lambda\><space|0.25spc>f<rsub|a*b>-<frac|1|4><space|0.25spc>\<partial\><rsub|a><wide|\<lambda\>|\<bar\>><space|0.25spc>\<gamma\><rsub|a><space|0.25spc>\<gamma\><rsub|b*c><space|0.25spc>\<epsilon\><space|0.25spc>f<rsub|b*c>;>

      \ 
    </output>

    <\textput>
      Now we do the remaining gamma matrix algebra.

      \;
    </textput>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      @join!(%){expand}: @distribute!(%): @eliminate_kr!(%);
    </input>

    <\output>
      S:= <with|mode|math|<frac|1|2><space|0.25spc><wide|\<epsilon\>|\<bar\>><space|0.25spc>\<gamma\><rsub|a><space|0.25spc>\<partial\><rsub|b>\<lambda\><space|0.25spc>f<rsub|a*b>-<frac|1|4><space|0.25spc><wide|\<epsilon\>|\<bar\>><space|0.25spc>\<gamma\><rsub|a*b*c><space|0.25spc>\<partial\><rsub|a>\<lambda\><space|0.25spc>f<rsub|b*c>-<frac|1|4><space|0.25spc>\<partial\><rsub|a><wide|\<lambda\>|\<bar\>><space|0.25spc>\<gamma\><rsub|a*b*c><space|0.25spc>\<epsilon\><space|0.25spc>f<rsub|b*c>-<frac|1|2><space|0.25spc>\<partial\><rsub|a><wide|\<lambda\>|\<bar\>><space|0.25spc>\<gamma\><rsub|b><space|0.25spc>\<epsilon\><space|0.25spc>f<rsub|a*b>;>

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      @substitute!(%)( \\partial_{a}{\\bar{\\lambda}} -\<gtr\>
      \\bar{\\partial_{a}{\\lambda}} );
    </input>

    <\output>
      S:= <with|mode|math|<frac|1|2><space|0.25spc><wide|\<epsilon\>|\<bar\>><space|0.25spc>\<gamma\><rsub|a><space|0.25spc>\<partial\><rsub|b>\<lambda\><space|0.25spc>f<rsub|a*b>-<frac|1|4><space|0.25spc><wide|\<epsilon\>|\<bar\>><space|0.25spc>\<gamma\><rsub|a*b*c><space|0.25spc>\<partial\><rsub|a>\<lambda\><space|0.25spc>f<rsub|b*c>-<frac|1|4><space|0.25spc><wide|\<partial\><rsub|a>\<lambda\>|\<bar\>><space|0.25spc>\<gamma\><rsub|a*b*c><space|0.25spc>\<epsilon\><space|0.25spc>f<rsub|b*c>-<frac|1|2><space|0.25spc><wide|\<partial\><rsub|a>\<lambda\>|\<bar\>><space|0.25spc>\<gamma\><rsub|b><space|0.25spc>\<epsilon\><space|0.25spc>f<rsub|a*b>;>

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      @spinorsort!(%);
    </input>

    <\output>
      S:= <with|mode|math|(<frac|-1|2>)<space|0.25spc><wide|\<epsilon\>|\<bar\>><space|0.25spc>\<gamma\><rsub|a*b*c><space|0.25spc>\<partial\><rsub|a>\<lambda\><space|0.25spc>f<rsub|b*c>;>

      \ 
    </output>

    <\input|<with|mode|math|color|red|\<gtr\>>>
      \;
    </input>
  </session>>
</body>

<\initial>
  <\collection>
    <associate|font-base-size|9>
    <associate|language|american>
    <associate|page-type|letter>
  </collection>
</initial>