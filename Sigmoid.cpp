#include "performance_test.h"
#include <iostream>
#include "math.h"
#include "sigmoid.h"

using namespace std;
using namespace seal;


/*
* This method performs homomorphic evaluation of least squares approximated sigmoid fucntion
* on input range [-8,8]
*/
void sigmoid_setup(shared_ptr<SEALContext> context, int q)
{
    chrono::high_resolution_clock::time_point time_start, time_end;
    chrono::microseconds time_diff(0);
    print_parameters(context);
    cout << endl;
    KeyGenerator keygen(context);
    auto public_key = keygen.public_key();
    auto secret_key = keygen.secret_key();
    auto relin_keys = keygen.relin_keys_local();
    Encryptor encryptor(context, public_key);
    Evaluator evaluator(context);
    Decryptor decryptor(context, secret_key);
    CKKSEncoder encoder(context);
    size_t slot_count = encoder.slot_count();
    double scale = pow(2.0, q);

    int count = 0;
    Plaintext plain_x, out_plain;
    Ciphertext enc_x, output;
    vector<double> result;
    double error[17];
    for (int i = 0; i < 17; i++)
    {
        double range = 8;
        double x = i - range;
        encoder.encode(x, scale, plain_x);
        encryptor.encrypt(plain_x, enc_x);
        time_start = chrono::high_resolution_clock::now();
        sigmoid(context, enc_x, output, q, relin_keys);
        time_end = chrono::high_resolution_clock::now();
        time_diff += chrono::duration_cast<chrono::microseconds>(time_end - time_start);

        decryptor.decrypt(output, out_plain);
        encoder.decode(out_plain, result);
        cout << endl;

        // HE error computation
        double b0 = 0.5, b1 = 1.96798, b3 = 5.43594, b5 = 7.48602, b7 = 3.56812;

        double original[5];
        double frac_x = x / 10;
        original[0] = b0;
        original[1] = b1 * frac_x;
        double cube = pow(frac_x, 3);
        original[2] = b3 * cube;
        double fifth = pow(frac_x, 5);
        original[3] = b5 * fifth;
        double seventh = pow(frac_x, 7);
        original[4] = b7 * seventh;
        original[5] = original[0] + original[1] - original[2] + original[3] - original[4];

        error[i] = result[0] - original[5];
        result.clear();
        count++;
    }

    cout << " + Size of sigmoid function output ciphertext is : " << (output.int_array().size()) / 1024 << " KB "
         << endl;
    cout << " + Average computation time for sigmoid calculation is : " << time_diff.count() / count
         << " microseconds. " << endl;
    cout << " + Error in Homomorphic evaluation of sigmoid function is : " << endl;
    for (int j = 1; j < 17; j++)
        error[0] += error[j];
    double mean_error = error[0] / 17;
    cout << mean_error;
    cout << endl;
    cout.flush();
}