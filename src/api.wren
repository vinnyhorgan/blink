//--------------------
// Graphics
//--------------------

class Graphics {
    foreign static clip(x, y, w, h)
    foreign static clear(color)
    foreign static get(x, y)
    foreign static set(x, y, color)
    foreign static line(x0, y0, x1, y1, color)
    foreign static fill(x, y, w, h, color)
    foreign static rectangle(x, y, w, h, color)
    foreign static fillRectangle(x, y, w, h, color)
    foreign static circle(x, y, r, color)
    foreign static fillCircle(x, y, r, color)
    foreign static blit(image, dx, dy, sx, sy, w, h)
    foreign static blitAlpha(image, dx, dy, sx, sy, w, h, alpha)
    foreign static blitTint(image, dx, dy, sx, sy, w, h, tint)
    foreign static print(text, x, y, color)
    foreign static print(font, text, x, y, color)
    foreign static screenshot()
    foreign static measure(text)

    static clip() {
        clip(0, 0, -1, -1)
    }

    static blit(image, x, y) {
        blit(image, x, y, 0, 0, image.width, image.height)
    }

    static blitAlpha(image, x, y) {
        blitAlpha(image, x, y, 0, 0, image.width, image.height, 1)
    }

    static blitAlpha(image, x, y, alpha) {
        blitAlpha(image, x, y, 0, 0, image.width, image.height, alpha)
    }

    static blitTint(image, x, y, tint) {
        blitTint(image, x, y, 0, 0, image.width, image.height, tint)
    }

    foreign static width
    foreign static height
    foreign static clearColor=(v)
}

foreign class Color {
    foreign construct new(r, g, b, a)
    foreign construct new(r, g, b)

    foreign [index]
    foreign [index]=(v)

    r { this[2] }
    g { this[1] }
    b { this[0] }
    a { this[3] }

    r=(v) { this[2] = v }
    g=(v) { this[1] = v }
    b=(v) { this[0] = v }
    a=(v) { this[3] = v }

    toString { "Color (r: %(r), g: %(g), b: %(b), a: %(a))" }

    static none { new(0, 0, 0, 0) }
    static black { new(0, 0, 0) }
    static darkBlue { new(29, 43, 83) }
    static darkPurple { new(126, 37, 83) }
    static darkGreen { new(0, 135, 81) }
    static brown { new(171, 82, 54) }
    static darkGray { new(95, 87, 79) }
    static lightGray { new(194, 195, 199) }
    static white { new(255, 241, 232) }
    static red { new(255, 0, 77) }
    static orange { new(255, 163, 0) }
    static yellow { new(255, 236, 39) }
    static green { new(0, 228, 54) }
    static blue { new(41, 173, 255) }
    static indigo { new(131, 118, 156) }
    static pink { new(255, 119, 168) }
    static peach { new(255, 204, 170) }
}

foreign class Image {
    foreign construct new(w, h)
    foreign construct new(filename)
    foreign construct fromMemory(data)

    foreign clip(x, y, w, h)
    foreign clear(color)
    foreign get(x, y)
    foreign set(x, y, color)
    foreign line(x0, y0, x1, y1, color)
    foreign fill(x, y, w, h, color)
    foreign rectangle(x, y, w, h, color)
    foreign fillRectangle(x, y, w, h, color)
    foreign circle(x, y, r, color)
    foreign fillCircle(x, y, r, color)
    foreign blit(image, dx, dy, sx, sy, w, h)
    foreign blitAlpha(image, dx, dy, sx, sy, w, h, alpha)
    foreign blitTint(image, dx, dy, sx, sy, w, h, tint)
    foreign print(text, x, y, color)
    foreign print(font, text, x, y, color)
    foreign resize(w, h)
    foreign save(type, filename)
    foreign saveToMemory()

    clip() {
        clip(0, 0, -1, -1)
    }

    blit(image, x, y) {
        blit(image, x, y, 0, 0, image.width, image.height)
    }

    blitAlpha(image, x, y) {
        blitAlpha(image, x, y, 0, 0, image.width, image.height, 1)
    }

    blitAlpha(image, x, y, alpha) {
        blitAlpha(image, x, y, 0, 0, image.width, image.height, alpha)
    }

    blitTint(image, x, y, tint) {
        blitTint(image, x, y, 0, 0, image.width, image.height, tint)
    }

    foreign width
    foreign height
}

foreign class Font {
    foreign construct new(filename)
    foreign construct fromMemory(data)

    foreign measure(text)
}

//--------------------
// Audio
//--------------------

foreign class Source {
    foreign construct new(filename)
    foreign construct fromSound(sound)

    foreign play()
    foreign pause()
    foreign stop()

    foreign length
    foreign position
    foreign state
    foreign gain=(v)
    foreign pan=(v)
    foreign pitch=(v)
    foreign loop=(v)
}

foreign class Sound {
    foreign construct new(samples, sampleRate, bitDepth, channels)
    foreign construct new(filename)
    foreign construct fromMemory(data)

    foreign getSample(index)
    foreign setSample(index, sample)
    foreign save(filename)
    foreign saveToMemory()

    getSample(index, channel) {
        getSample(index * channels + (channel - 1))
    }

    setSample(index, channel, sample) {
        setSample(index * channels + (channel - 1), sample)
    }

    foreign bitDepth
    foreign sampleRate
    foreign channels
    foreign length
}

//--------------------
// Input
//--------------------

class Keyboard {
    foreign static down(key)
    foreign static pressed(key)
}

class Mouse {
    foreign static down(button)
    foreign static pressed(button)

    foreign static x
    foreign static y
    foreign static scrollX
    foreign static scrollY
}

//--------------------
// System
//--------------------

class Window {
    foreign static close()

    foreign static active
    foreign static width
    foreign static height
}

class OS {
    foreign static name
    foreign static blinkVersion
    foreign static args
    foreign static clipboard
    foreign static clipboard=(v)
}

class Directory {
    foreign static exists(path)
    foreign static list(path)
}

class File {
    foreign static exists(path)
    foreign static size(path)
    foreign static modTime(path)
    foreign static read(path)
    foreign static write(path, data)
}

foreign class Request {
    foreign construct new(url)

    foreign make()

    foreign complete
    foreign status
    foreign body
}

//--------------------
// Base Game
//--------------------

class BaseGame {
    config(t) {}
    init() {}
    update(dt) {}
    draw() {}
    active(active) {}
    resize(width, height) {}
    close() {}
    keyboard(key, pressed) {}
    input(char) {}
    mouseButton(button, pressed) {}
    mouseMove(x, y) {}
    mouseScroll(dx, dy) {}
    drop(paths) {}
}
