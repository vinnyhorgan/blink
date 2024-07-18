import "blink" for BaseGame, Color, Graphics, Image, Mouse
import "random" for Random

class Vector {
    construct new(x, y) {
        _x = x
        _y = y
    }

    x { _x }
    y { _y }
    x=(x) { _x = x }
    y=(y) { _y = y }
}

class Squinkle {
    construct new(pos, spd, col) {
        _pos = pos
        _spd = spd
        _col = col
    }

    pos { _pos }
    spd { _spd }
    col { _col }
}

class Game is BaseGame {
    construct new() {}

    init() {
        _random = Random.new()
        _squinkleImage = Image.new("squinkle.png")
        _squinkles = []

        Graphics.clearColor = Color.white
    }

    update(dt) {
        _fps = 1.0 / dt

        if (Mouse.down(1)) {
            for (i in 0..100) {
                var newSquinkle = Squinkle.new(
                    Vector.new(Mouse.x, Mouse.y),
                    Vector.new(_random.float(-250, 250) / 60, _random.float(-250, 250) / 60),
                    Color.new(_random.int(50, 240), _random.int(80, 240), _random.int(100, 240))
                )

                _squinkles.add(newSquinkle)
            }
        }

        for (s in _squinkles) {
            s.pos.x = s.pos.x + s.spd.x
            s.pos.y = s.pos.y + s.spd.y

            if (((s.pos.x + _squinkleImage.width / 2) > Graphics.width) || ((s.pos.x + _squinkleImage.width / 2) < 0)) {
                s.spd.x = -s.spd.x
            }

            if (((s.pos.y + _squinkleImage.height / 2) > Graphics.height) || ((s.pos.y + _squinkleImage.height / 2 - 28) < 0)) {
                s.spd.y = -s.spd.y
            }
        }
    }

    draw() {
        for (s in _squinkles) {
            Graphics.blitTint(_squinkleImage, s.pos.x, s.pos.y, s.col)
        }

        Graphics.fill(0, 0, Graphics.width, 28, Color.black)

        Graphics.print("FPS: %(_fps.floor)", 10, 10, Color.white)
        Graphics.print("Squinkles: %(_squinkles.count)", 100, 10, Color.white)
    }
}

var main = Game.new()
