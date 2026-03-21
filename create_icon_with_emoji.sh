#!/bin/bash

echo "🍅 创建番茄图标..."

# 使用 Python 和 Quartz 2D 创建番茄图标
python3 << 'PYEOF'
from AppKit import NSImage, NSBezierPath, NSColor
from CoreImage import CIImage
from Foundation import NSRect, NSPoint, NSSize

def create_tomato_icon(size):
    # 创建图像
    image = NSImage.alloc().initWithSize_(NSMakeSize(size, size))
    
    image.lockFocus()
    
    # 获取图形上下文
    from AppKit import NSGraphicsContext
    ctx = NSGraphicsContext.currentContext()
    cgctx = ctx.CGContext()
    
    # 设置颜色
    red = NSColor.colorWithRed_green_blue_alpha_(1.0, 0.34, 0.13, 1.0)
    green = NSColor.colorWithRed_green_blue_alpha_(0.3, 0.69, 0.31, 1.0)
    dark_red = NSColor.colorWithRed_green_blue_alpha_(0.9, 0.5, 0.13, 1.0)
    
    center_x = size / 2
    center_y = size / 2
    
    # 绘制番茄身体（椭圆形）
    body_width = size * 0.5
    body_height = size * 0.6
    body_rect = NSRect(center_x - body_width/2, center_y - body_height*0.4, body_width, body_height*0.7)
    
    NSColor.red.set()
    ctx.CGContext().fillEllipseInRect(body_rect)
    
    # 绘制番茄顶部（绿色椭圆）
    top_height = body_height * 0.3
    top_rect = NSRect(center_x - body_width/2, center_y - body_height*0.4, body_width, top_height)
    
    NSColor.greenColor().set()
    ctx.CGContext().fillEllipseInRect(top_rect)
    
    # 绘制左叶子
    leaf_width = size * 0.15
    leaf_height = size * 0.1
    leaf_rect = NSRect(center_x + body_width*0.25, center_y - body_height*0.4, leaf_width, leaf_height)
    
    # 旋转叶子
    from Foundation import NSAffineTransform
    transform = NSAffineTransform.alloc().init()
    transform.rotateByDegrees_(-30)
    
    NSColor.greenColor().set()
    ctx.CGContext().saveGState()
    ctx.CGContext().translateCTM(center_x + body_width*0.25, center_y - body_height*0.4)
    ctx.CGContext().rotateCTM(-30 * 3.14159 / 180)
    ctx.CGContext().translateCTM(-(center_x + body_width*0.25), -(center_y - body_height*0.4))
    ctx.CGContext().fillEllipseInRect(leaf_rect)
    ctx.CGContext().restoreGState()
    
    # 绘制右叶子
    leaf_rect2 = NSRect(center_x + body_width*0.1, center_y - body_height*0.55, leaf_height, leaf_width)
    ctx.CGContext().saveGState()
    ctx.CGContext().translateCTM(center_x + body_width*0.1, center_y - body_height*0.55)
    ctx.CGContext().rotateCTM(30 * 3.14159 / 180)
    ctx.CGContext().translateCTM(-(center_x + body_width*0.1), -(center_y - body_height*0.55))
    ctx.CGContext().fillEllipseInRect(leaf_rect2)
    ctx.CGContext().restoreGState()
    
    image.unlockFocus()
    
    return image

# 创建 1024x1024 的图标
icon = create_tomato_icon(1024)

# 保存为 PNG
tiff = icon.TIFFRepresentation()
bitmap = NSBitmapImageRep.imageRepWithData_(tiff)
png = bitmap.representationUsingType_properties_(0, None)

png.writeToFile_atomically_('icon.png', False)
print("图标已保存为 icon.png")
PYEOF
