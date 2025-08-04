from PIL import Image

img = Image.open("known.jpg").convert("RGB")
img.save("known.jpg")  # Overwrites it in correct format
print("✅ known.jpg converted to RGB and saved.")
