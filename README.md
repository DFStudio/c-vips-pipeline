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

For optional values, an underscore (`_`) represents no value.

```
@thumbnail <file in = 0> <slot out = 1> <width = 2> <height? = 3> <no-rotate = 4> <intent? = 5>
@profile <slot in = 0> <slot out = 1> <profile = 2>
@unsharp <slot in = 0> <slot out = 1> <sigma = 2> <strength = 3>

@load <file in = 0> <slot out = 1>
@composite <base slot = 0> <overlay slot = 1> <slot out = 2> <overlay x = 3> <overlay y = 4> <blend mode = 5>
@extend <slot in = 0> <slot out = 1> <x = 2> <y = 3> <width = 4> <height = 5> <extend = 6> // vips_embed
 
@flatten <slot in = 0> <slot out = 1> <background red = 2> <background green = 3> <background blue = 4>
@add_alpha <slot in = 0> <slot out = 1> // vips_addalpha

@write <slot in = 0> <file out = 1>
@free <slot = 0>
```
