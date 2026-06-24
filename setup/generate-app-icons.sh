#!/bin/bash

#Script to generate all required app icon sizes from a master PNG

if [ $# -eq 0 ]; then
	echo "Usage: ./generate-app-icons.sh <input-png-file>"
	echo "Example: ./generate-app-icons.sh icon-1080p.png"
	exit 1
fi

INPUT_FILE="$1"

#Check if input file exists
if [ ! -f "$INPUT_FILE" ]; then
	echo "Error: Input file '$INPUT_FILE' not found"
	exit 1
fi

#Check if ImageMagick is installed
if ! command -v magick &> /dev/null; then
	echo "Error: ImageMagick is not installed. Install it with: brew install imagemagick"
	exit 1
fi

ICON_DIR="Assets.xcassets/AppIcon.appiconset"

#Create directory if it doesn't exist
mkdir -p "$ICON_DIR"

#Array of all required sizes (unique pixel dimensions)
declare -a SIZES=(
	16 20 24 27.5 29 32 33 40 44 46 48 50 51 54 55
	58 60 64 66 72 76 80 83.5 86 87 88 92 98 100 102
	108 117 120 128 129 144 152 167 172 180 196 216 234
	256 258 512 1024
)

echo "Generating app icons from $INPUT_FILE..."

#Generate each size
for SIZE in "${SIZES[@]}"; do
	#Handle floating point sizes (27.5, 83.5)
	PIXEL_SIZE=$(echo "$SIZE * 1" | bc)
	FILENAME="$ICON_DIR/$(printf "%.0f" "$SIZE").png"
	
	#Use the size as integer for convert command (ImageMagick handles this)
	CONVERT_SIZE="${SIZE%.*}x${SIZE%.*}"
	
	#For floating point sizes, round up
	if [[ "$SIZE" == *"."* ]]; then
		CONVERT_SIZE=$(printf "%.0f" "$SIZE")
		CONVERT_SIZE="${CONVERT_SIZE}x${CONVERT_SIZE}"
	fi
	
	#Generate the icon
	magick convert "$INPUT_FILE" -resize "$CONVERT_SIZE" -gravity center -background transparent -extent "$CONVERT_SIZE" "$FILENAME"
	
	if [ $? -eq 0 ]; then
		echo "✓ Generated $FILENAME (${SIZE}×${SIZE})"
	else
		echo "✗ Failed to generate $FILENAME"
		exit 1
	fi
done

echo ""
echo "Success! Generated all app icon sizes in $ICON_DIR"
echo "The Contents.json file already contains the correct mappings."
