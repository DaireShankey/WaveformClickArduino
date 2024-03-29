#include <SPI.h>//spi communication

// Define pins and constants
const int FSYNC_PIN = 10; // AD9833 FSYNC defined as pin 10 on arduino
const int CS_PIN_AD5227 = 2; // AD5227 Chip Select defined as pin 2 on arduino
const double MASTER_CLK = 25e6; // Master clock frequency for AD9833

class AD9833SineWave {
public:
    explicit AD9833SineWave(int fsyncPin) : fsyncPin(fsyncPin) { //constructor for sinewave class
        pinMode(fsyncPin, OUTPUT);
        digitalWrite(fsyncPin, HIGH); // De-select the AD9833 initially
    }

    void begin() {
        SPI.begin();
        SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE2)); // Lower SPI speed for stability
        writeRegister(0x2100); // Reset the AD9833
        delay(10); // Wait for the reset to take effect
        SPI.endTransaction();
    }

    void setFrequency(double frequency) {
        unsigned int freqValue = static_cast<unsigned int>((frequency * (1ULL << 28)) / MASTER_CLK);
        unsigned int LSB = freqValue & 0x3FFF; // Lower 14 bits
        unsigned int MSB = (freqValue >> 14) & 0x3FFF; // Upper 14 bits

        SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE2));
        writeRegister(0x2100); // Reset command to ensure clean start
        writeRegister(0x4000 | LSB); // Set the lower part of the frequency
        writeRegister(0x4000 | MSB); // Set the upper part of the frequency
        writeRegister(0xC000); // Default phase register (optional, for phase adjustment if needed)
        writeRegister(0x2000); // Exit from Reset
        SPI.endTransaction();
    }

private:
    int fsyncPin;

    void writeRegister(unsigned int data) {
        digitalWrite(fsyncPin, LOW); // Enable communication with AD9833
        SPI.transfer(highByte(data)); // Send higher 8 bits
        SPI.transfer(lowByte(data)); // Send lower 8 bits
        digitalWrite(fsyncPin, HIGH); // Finish communication
    }
};

AD9833SineWave sineWave(FSYNC_PIN); // Create an AD9833 control object

void setup() {
    Serial.begin(9600); // Start serial for debugging
    pinMode(CS_PIN_AD5227, OUTPUT);
    digitalWrite(CS_PIN_AD5227, HIGH); // Ensure AD5227 is deselected

    sineWave.begin(); // Initialize the AD9833 module
    sineWave.setFrequency(1000000); // Set output frequency to 1 MHz

    // Set AD5227 to min resistance to achieve maximum amplitude
    // Change this value to vary resistance ,0 being max resistance and 63 being minimum resistance 
    setAD5227Position(63); // Set to minimum resistance for max amplitude
}

void loop() {
    // The waveform is continuously output at the set frequency and amplitude.
    // There is no need to update the amplitude or frequency in this loop.
    delay(1000); // Placeholder delay
}

void setAD5227Position(int position) {
    position = constrain(position, 0, 63); // Ensure the position is within the valid range
    SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE2));
    digitalWrite(CS_PIN_AD5227, LOW); // Select the AD5227
    SPI.transfer(position << 2); // Send the position with alignment to the MSBs for AD5227
    digitalWrite(CS_PIN_AD5227, HIGH); // Deselect AD5227
    SPI.endTransaction();
}
