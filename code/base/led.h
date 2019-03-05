// library interface description
class Led
{
    // user-accessible "public" interface
  public:
    Led() {};
    void begin(int pin, bool defaultValue) {
      _pin = pin;
      _defaultValue = defaultValue;
      pinMode(_pin, OUTPUT);
      digitalWrite(_pin, _defaultValue);
    };
    void begin(int pin) {
      begin(pin, LOW);
    };
    void on() {
      digitalWrite(_pin, !_defaultValue);
    };
    void off() {
      digitalWrite(_pin, _defaultValue);
    };

    // library-accessible "private" interface
  private:
    int _pin = LED_BUILTIN;
    bool _defaultValue = LOW;
};
