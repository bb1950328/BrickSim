import glob
import os.path
import pathlib
import re

import cairosvg

GLOBS = [
    ["./resources/icons/*.svg", "./resources/icons/16x16/*.png", [16, 16]],
    ["./resources/icons/*.svg", "./resources/icons/36x36/*.png", [36, 36]],
    ["./resources/icons/*.svg", "./resources/icons/48x48/*.png", [48, 48]],
]


def convert_all() -> None:
    for in_pattern, out_pattern, (img_width, img_height) in GLOBS:
        for svg_path in glob.iglob(in_pattern):
            png_path = pathlib.Path(re.sub(in_pattern.replace("*", "(.*)"), out_pattern.replace("*", "\\1"), svg_path))
            png_path.parent.mkdir(parents=True, exist_ok=True)
            print(f"convert {svg_path} to {png_path}")
            cairosvg.svg2png(url=svg_path, write_to=str(png_path), output_width=img_width, output_height=img_height)
            # with cairo.SVGSurface(svg_path, 32, 32) as surface:
            #     surface.write_to_png(png_path)


def check_cwd():
    if not os.path.exists("./resources"):
        raise FileNotFoundError("set the working directory to the workspace root")


if __name__ == '__main__':
    check_cwd()
    convert_all();
