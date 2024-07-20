import "blink" for BaseGame, Color, Font, Graphics

class Game is BaseGame {
    construct new() {}

    init() {
        _font = Font.new("font.png")

        Graphics.clearColor = Color.white
    }

    draw() {
        Graphics.print("Hi! I'm the default font!", 10, 10, Color.black)
        Graphics.print(_font, "Hi! I'm a different font!", 10, 30, Color.black)
    }
}

var main = Game.new()
