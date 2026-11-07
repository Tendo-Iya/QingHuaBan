from PIL import Image, ImageDraw, ImageFont, ImageFilter

S = 512
img = Image.new('RGBA', (S, S), (0, 0, 0, 0))
mask = Image.new('L', (S, S), 0)
md = ImageDraw.Draw(mask)
md.rounded_rectangle((16, 16, S - 16, S - 16), radius=118, fill=255)
mask = mask.filter(ImageFilter.GaussianBlur(1))

grad = Image.new('RGBA', (S, S), (0, 0, 0, 0))
px = grad.load()
for y in range(S):
    for x in range(S):
        t = max(0, min(1, (x * 0.35 + y * 0.65) / S))
        r = int(42 * (1 - t) + 11 * t)
        g = int(211 * (1 - t) + 143 * t)
        b = int(166 * (1 - t) + 105 * t)
        px[x, y] = (r, g, b, 255)
img.paste(grad, (0, 0), mask)

d = ImageDraw.Draw(img)
d.rounded_rectangle((18, 18, S - 18, S - 18), radius=116, outline=(255, 255, 255, 110), width=5)
d.arc((40, 20, S - 40, 290), 200, 340, fill=(255, 255, 255, 90), width=8)

glow = Image.new('RGBA', (S, S), (0, 0, 0, 0))
gd = ImageDraw.Draw(glow)
gd.ellipse((70, 60, S - 70, 360), fill=(255, 255, 255, 35))
glow = glow.filter(ImageFilter.GaussianBlur(55))
img = Image.alpha_composite(img, glow)

d = ImageDraw.Draw(img)
font = ImageFont.truetype(r'C:\Windows\Fonts\segoeuib.ttf', 300)
text = 'H'
bbox = d.textbbox((0, 0), text, font=font)
tw, th = bbox[2] - bbox[0], bbox[3] - bbox[1]
d.text(((S - tw) / 2 - bbox[0], (S - th) / 2 - bbox[1] - 10), text, font=font, fill=(255, 255, 255, 245))
d.ellipse((S - 142, 82, S - 96, 128), fill=(255, 255, 255, 215))
d.ellipse((S - 132, 92, S - 106, 118), fill=(112, 246, 209, 255))

base = r'C:\Users\15797\Documents\deepseek-harness\default-workspace\QingHuaBan\assets'
img.save(base + r'\app.png')
sizes = [(256, 256), (128, 128), (64, 64), (48, 48), (32, 32), (16, 16)]
img.save(base + r'\app.ico', sizes=sizes)
print('icon generated')
