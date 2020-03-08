echo "Start build BEngine project"
echo "Make windows proejct"

cd windows
eho %PWD%
cmake .. -DWindwos=1

cd ../android
cmake -DAndroid=1
