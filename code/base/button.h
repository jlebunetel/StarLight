// library interface description
class Button
{
    // user-accessible "public" interface
  public:
    Button() {};
    void begin(int pin, bool defaultStatus) {
      _pin = pin;
      _status = defaultStatus;
      _previousStatus = defaultStatus;
      _defaultStatus = defaultStatus;
      pinMode(_pin, INPUT);
    };
    void begin(int pin) {
      begin(pin, LOW);
    };
    bool read() {
      _previousStatus = _status;
      _status = digitalRead(_pin);
      return _status;
    };
    bool isOn() {
      return (_status != _defaultStatus);
    };
    bool isOff() {
      return (_status == _defaultStatus);
    };
    bool hasChanged() {
      return (_status != _previousStatus);
    };
    bool isJustOn() {
      return (hasChanged() && isOn());
    };
    bool isJustOff() {
      return (hasChanged() && isOff());
    };

    // library-accessible "private" interface
  private:
    int _pin = 0;
    bool _status = LOW;
    bool _previousStatus = LOW;
    bool _defaultStatus = LOW;
};
