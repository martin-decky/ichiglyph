# Ichiglyph language tools

These are the Ichiglyph language tools. Ichiglyph is a simple Turing-complete
language derived from [Brainfuck](https://en.wikipedia.org/wiki/Brainfuck).

Ichiglyph and Brainfuck implement the same set of instructions, the only major
difference is their encoding. The following table presents the encoding of the
instructions of both languages.

| Brainfuck | Ichiglyph |
| --------- | --------- |
| `>`       | `ll`      |
| `<`       | `lI`      |
| `+`       | `Il`      |
| `-`       | `II`      |
| `.`       | `1l`      |
| `,`       | `1I`      |
| `[`       | `l1`      |
| `]`       | `I1`      |

The Ichiglyph language was inspired by a remark by Josefina Mádrová, who
cleverly noted that using the characters `l`, `I` and `1` in identifiers is
a bad practice.

The following tools are available:

 * [ichiglyph.c](interpreter/ichiglyph/ichiglyph.c): Ichiglyph interpreter
 * [brainfuck.c](interpreter/brainfuck/brainfuck.c): Brainfuck interpreter (as a reference)
 * [bf2ig.c](transpiler/bf2ig/bf2ig.c): Brainfuck to Ichiglyph transpiler
 * [ig2bf.c](transpiler/ig2bf/ig2bf.c): Ichiglyph to Brainfuck transpiler

There are also several Brainfuck and equivalent Ichiglyph sample programs in
the `examples` directory. The original Brainfuck programs were taken directly
from [pablojorge's GitHub repo](https://github.com/pablojorge/brainfuck).
These programs are copyrighted by their respective authors.

## What is the use of this?

As with Brainfuck itself and all its variants and derivatives, the purpose is
mostly just to have fun. However, Ichiglyph programs could be used to
demonstrate that certain fonts are unsuitable for rendering source code.

![I and l indistinguishable](examples/hello.png "I and l indistinguishable")
![l and 1 hardly distinguishable](examples/hello_vsc.png "l and 1 hardly distinguishable")
![all characters somewhat distinguishable](examples/hello_fixed.png "all characters somewhat distinguishable")

## What does the name mean?

**Ichi** means **one** in Japanese. **Glyph** is an elemental symbol within an
agreed set of symbols.
