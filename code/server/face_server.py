from PIL import Image
# Assuming the correct path to the image
known_faces = 'C:/Users/veera/OneDrive/Desktop/ESP32_FaceServer/veer.jpeg'  # Use the correct full path here
main(known_faces)

def convert_to_gray(known_faces, save_path):
    # Open the image file
    img = Image.open(known_faces)
    
    # Convert image to 8-bit grayscale
    gray_img = img.convert("L")
    
    # Save the grayscale image
    gray_img.save(save_path)
    print(f"Image converted to 8-bit grayscale and saved to {save_path}")

def convert_to_rgb(known_faces, save_path):
    # Open the image file
    img = Image.open(known_faces)
    
    # Convert image to RGB
    rgb_img = img.convert("RGB")
    
    # Save the RGB image
    rgb_img.save(save_path)
    print(f"Image converted to RGB and saved to {save_path}")

# Example usage
known_faces = "C:\Users\veera\OneDrive\Desktop\ESP32_FaceServer\known_faces"  # Replace with your input file
gray_save_path = "output_gray.jpg"  # Replace with desired output path for grayscale
rgb_save_path = "output_rgb.jpg"  # Replace with desired output path for RGB

# Convert to grayscale
convert_to_gray(known_faces, gray_save_path)

# Convert to RGB
convert_to_rgb(known_faces, rgb_save_path)
