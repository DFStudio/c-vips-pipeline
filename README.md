# c-vips-pipeline

## About The Project

This is a simple application to interact with the VIPS API in order to perform a simple scale
of an input image and subsequently perform sharpening on the scaled image before output. The
motivation for creating this simple application rather than simply using the `vipsthumbnail`
or `vips` is: (1) `vipsthumbnail` does not provide control over the sharpening used to create
its output thumbnails, and (2) the `vips` CLI requires creating intermediate files on disk in
order to make multiple calls to the VIPS API, which would negatively affect performance
dramatically.

## Commands

- For optional values, an underscore (`_`) represents no value.
- Any numeric parameter can be defined using a variable or expression by prefixing it with a `%`. Expressions are evaluated using [ExprTk](https://github.com/ArashPartow/exprtk).
- Variables are all internally represented with 64-bit floating point numbers
- Slots and variables use string keys

```
@thumbnail <file in> <slot out> <width> <height?> <no-rotate> <intent?>
@profile <slot in> <slot out> <profile>
@unsharp <slot in> <slot out> <sigma> <strength>

@load <file in> <slot out>
@composite <base slot> <overlay slot> <slot out> <overlay x> <overlay y> <blend mode>
@embed <slot in> <slot out> <x> <y> <width> <height> <extend> <bg red> <bg green> <bg blue> <bg alpha>
 
@flatten <slot in> <slot out> <background red> <background green> <background blue>
@add_alpha <slot in> <slot out> <alpha>
@scale <slot in> <slot out> <hscale> <vscale>
@affine <slot in> <slot out> <m00> <m01> <tx> <m10> <m11> <ty>
@fit <slot in> <slot out> <width?> <height?>
@trim_alpha <slot in> <slot out> <threshold> <margin>
@multiply_color <slot in> <slot out> <r> <g> <b> <a>

@stream <slot> <format>
@write <slot in> <file out>
@consume <slot>
@free <slot>
@copy_slot <source slot> <dest slot>

@set_var <var> <value>
```

Additional expression functions:
```
vips_width('name')
vips_height('name')
```
