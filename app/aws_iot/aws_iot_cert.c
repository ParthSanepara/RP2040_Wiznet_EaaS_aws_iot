#include "aws_iot_cert.h"

const uint8_t AWS_IOT_ROOT_CA[] =                                               \
    "-----BEGIN CERTIFICATE-----\r\n"                                           \
    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\r\n"      \
    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\r\n"      \
    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\r\n"      \
    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\r\n"      \
    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\r\n"      \
    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\r\n"      \
    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\r\n"      \
    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\r\n"      \
    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\r\n"      \
    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\r\n"      \
    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\r\n"      \
    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\r\n"      \
    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\r\n"      \
    "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\r\n"      \
    "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\r\n"      \
    "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\r\n"      \
    "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\r\n"      \
    "rqXRfboQnoZsG4q5WTP468SQvvG5\r\n"                                          \
    "-----END CERTIFICATE-----\r\n";

const uint8_t AWS_IOT_CLIENT_CERT[] =                                       \
    "-----BEGIN CERTIFICATE-----\r\n"                                       \
    "MIIDWTCCAkGgAwIBAgIUQeXpZCnT7zsM0xAsMvx6e7vwP6UwDQYJKoZIhvcNAQEL\r\n"  \
    "BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\r\n"  \
    "SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTIzMDEyNjA3MDQy\r\n"  \
    "NloXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\r\n"  \
    "ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKULFSn+OVKNj8Sa1t0f\r\n"  \
    "qkIOn0fpweqErsKLkSUA8oN5pk1HEDE9SP7AB9NAYsR8Y4WPUcwCrQC7CdrM7fi9\r\n"  \
    "Zr8OXyC4dIABBznignhlNcyi3OdYBYKS1fC/G+82XGtNkniiclZsFaiM3wZDt/yo\r\n"  \
    "z2ZX+h4iXmzJ8fN8u2HD17B/dMtVI7MRB4COfoE5Q344lx1VRrmN2Br3AtTQMjKE\r\n"  \
    "X5Iqt/xV+ALZCIPKxiiZZ8eBh0TmITn+3PYx5069Iv0AD80CFCES+srjVvcz+sLG\r\n"  \
    "r6Jr+cko4R9YWEforoMBRbPjQ2tdB5XDWrTs8WyrDIVfyzE00o7K26iFhBiwfBHo\r\n"  \
    "05ECAwEAAaNgMF4wHwYDVR0jBBgwFoAUsGqcdQmOk/+5QYdk9lSAyfEC8twwHQYD\r\n"  \
    "VR0OBBYEFMpz4DtWhYSpbL2YSHmc9NgvCNzEMAwGA1UdEwEB/wQCMAAwDgYDVR0P\r\n"  \
    "AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQCj6ERJ0JalB40PDEa9Ug0O7wed\r\n"  \
    "iSgu6Ok6yXPebRebpFuwsCAz9Y5sHv2C5ikrrOT27xxI3aw4CVnRe8jEAmEJpKsJ\r\n"  \
    "odrSpWarR45HMsv4CIJQdIfthZN9IWHEmHciyqtY1a9bkvqoh4YO9thu7QxRelRW\r\n"  \
    "qjgLInj9nQDJQYv/k9SaL1y+3kdD1nrl0b59Buc+FdT6Sd78tBC5j/Ph+bIKolx9\r\n"  \
    "2UQV9uaMwXCh20N4mr56Xwr+R/vjmIBHKsS4ur78Knc2cuxbMeO90kWFxiVauXAr\r\n"  \
    "b7eNoU8XIm99WvQNweqFNq6leW8iHkJ8XvTDO7eYyXCb6l+Ycpff4jZhOl36\r\n"      \
    "-----END CERTIFICATE-----\r\n";

const uint8_t AWS_IOT_PRIVATE_KEY[] =                                       \
    "-----BEGIN RSA PRIVATE KEY-----\r\n"                                   \
    "MIIEpAIBAAKCAQEApQsVKf45Uo2PxJrW3R+qQg6fR+nB6oSuwouRJQDyg3mmTUcQ\r\n"  \
    "MT1I/sAH00BixHxjhY9RzAKtALsJ2szt+L1mvw5fILh0gAEHOeKCeGU1zKLc51gF\r\n"  \
    "gpLV8L8b7zZca02SeKJyVmwVqIzfBkO3/KjPZlf6HiJebMnx83y7YcPXsH90y1Uj\r\n"  \
    "sxEHgI5+gTlDfjiXHVVGuY3YGvcC1NAyMoRfkiq3/FX4AtkIg8rGKJlnx4GHROYh\r\n"  \
    "Of7c9jHnTr0i/QAPzQIUIRL6yuNW9zP6wsavomv5ySjhH1hYR+iugwFFs+NDa10H\r\n"  \
    "lcNatOzxbKsMhV/LMTTSjsrbqIWEGLB8EejTkQIDAQABAoIBAQCWmH4/SxHTGgjN\r\n"  \
    "RNw5+OLKU71TSSEAdQFh0ygls6+V+DKe4PENfPZWPbLmOCHnV/kXrabdmLNzrzdW\r\n"  \
    "xxd9mJVgUCTNHoJvKoEFeIHEidWFXDZlghPFCWbdvgQnSlsaLjYwjUn3Qbihi6kV\r\n"  \
    "dkUz7nUQ1cYMKSAv9DUqD+6LTsBI8wycYSc3ZZsIlvgdyQ71t9LgOev32WGITTRH\r\n"  \
    "IbPRUhod77kQhvDCFAJECz2ySyLkk7PS6baZyDgXyVuednekxo4F6YQNchqpWT/A\r\n"  \
    "/xs/XL0Of8WxHqtQLERgbibwvklRrbL5ISMdhvq5x6fGQvyrWRUkCUxtJJYf+yZD\r\n"  \
    "epgEcbYBAoGBANXodURQWeyRKh4aI/siZb8dsbad7dOdRvL1CbUuZeJQxRyDa4pS\r\n"  \
    "AWl2nVBOdCX240eiVeGgpxZz6wy90ewTez/w8KCh8f/4fpOGqziWqqoqGdT7x64j\r\n"  \
    "AR3wDCWKUb43YK1exOFZ6mfoHxVpmCeJGncA6p+KgBpGBkzdWy/+7PgpAoGBAMWF\r\n"  \
    "FIekJD3XTrqooLfW6g+dxfa0sD60WiZSaQxugC7CkhCZAW/yXx651HGU7YOjy1EJ\r\n"  \
    "5hDwXuzU8apqNdBK9etgIONr/gv/0sL8GP2h679rhqCRoiXOLUocdypIJtazcoEP\r\n"  \
    "fN7D1dSxduX8qF9KVJnlBUHRf6dfERVVJvIB4w0pAoGAQ87ftZsfFm3iAw7YTxjV\r\n"  \
    "ViMsna4soQNfZU8mF6gwpfBiu6oxYfSi+/Kc5cMo8Iv1+lRMkyyhKu3uGejmbcaz\r\n"  \
    "ME7JiLIti64TOeLbziF80lpsO+bgoqP4C6x3vUK+rOTTIcJD+QfO1cdwaxJBKG8+\r\n"  \
    "03CX76ExAKMoBh+5c1qFdhkCgYAJBgVkFPma4aU1HdMJ0wWqZPjQRnzuwGqF6anm\r\n"  \
    "6X1tpXHq1DKenKH/01mDPtyC8Q6uyxb5myGbDDdmI6gFtgb6E9As05gtkChexAmY\r\n"  \
    "PLd6bT0fRMf1li+3fQlsoxMzJ8uJXd3Eh4nTR7A43YwLnmezCCVvVlTQnqU3Jka7\r\n"  \
    "M57d0QKBgQDHHKci/T8hi5H+mZfwt9ttF5av7VC9y82WGss/uOk/1DK9yEPjpySf\r\n"  \
    "blavmds/jiSwK35hh0OCb9WLw2LkLj0kK9Nb7+Cej6BRoU9hJMh++tM+Hv5jIeWD\r\n"  \
    "OgzkmZTctbAlXRvDassHWcRQAkImC8+luS3s1/DngGH0jrZYrya0Bw==\r\n"  \
    "-----END RSA PRIVATE KEY-----\r\n";
