from PIL import Image
import argparse

def resize_image(input_image_path, output_image_path, new_width, new_height):
    original_image = Image.open(input_image_path)
    resized_image = original_image.resize((new_width, new_height))
    resized_image.save(output_image_path)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Resize an image')
    parser.add_argument('-i', '--input', type=str, help='The input image')
    parser.add_argument('-o', '--output', type=str, help='The output image')
    parser.add_argument('-m', '--mode', type=str, default='w', help='by width or height')
    parser.add_argument('-p', '--pixels', type=int, default=500, help='pixels number of output')
    args = parser.parse_args()

    original_image = Image.open(args.input)
    ratio = original_image.size[1] / original_image.size[0]

    if args.mode == 'w':
        new_width = args.pixels
        new_height = int(args.pixels * ratio)
    elif args.mode == 'h':
        new_height = args.pixels
        new_width = int(args.pixels / ratio)
    else:
        print('Invalid mode; please choose "w" or "h"')
        exit()

    resized_image = original_image.resize((new_width, new_height))
    resized_image.save(args.output)
