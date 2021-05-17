#include "performance_test.h"
#include <iostream>
#include "math.h"
#include "neural_nw.h"

using namespace std;
using namespace seal;


/*
*This method performs homomorphic validation of deep neural network
* with pre-trained weights and biases. It comprises 1 input layer, 1 hidden layer
* and 1 output layer. It performs non-linear classification using sigmoid activation function.
*/

void dnn(shared_ptr<SEALContext> context, int q)
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

    vector<double> x1;
    vector<double> x2;
    vector<int> y;
    vector<int> y_pred;

    ifstream myFile("dnn.csv");
    int count = 0;
    vector<string> row;
    string line, word, temp;
    double csv_x1, csv_x2, csv_a1, csv_a2, csv_a3;
    int csv_y;

    if (myFile.good())
    {
        while (std::getline(myFile, line))
        {
            row.clear();
            stringstream s(line);

            while (std::getline(s, word, ','))
            {
                count++;
                row.push_back(word);
            }
            csv_x1 = stod(row[0]);
            csv_x2 = stod(row[1]);
            csv_y = stoi(row[5]);
            csv_a1 = stod(row[2]);
            csv_a2 = stod(row[3]);
            csv_a3 = stod(row[4]);

            if ((csv_a1 <= 8.0 && csv_a1 >= -8.0) && (csv_a2 <= 8.0 && csv_a2 >= -8.0) &&
                (csv_a3 <= 8.0 && csv_a3 >= -8.0))
            {
                x1.push_back(csv_x1);
                x2.push_back(csv_x2);
                y.push_back(csv_y);
            }
        }
    }

    myFile.close();

    double weight1 = -0.40623956138470096;
    double weight2 = 1.4836620748420632;
    double weight3 = -1.9713767969685267;
    double weight4 = 2.789910806269761;
    double weight_out1 = 8.65565967308743;
    double weight_out2 = -7.968147975662568;
    double bias1 = -9.158888066660062;
    double bias2 = -4.371639350992054;
    double bias_out = 3.708614314955103;

    Plaintext weight1_plain, weight2_plain, weight3_plain, weight4_plain, weight_out1_plain, weight_out2_plain,
        bias1_plain, bias2_plain, bias_out_plain;
    Plaintext input_plain1, input_plain2;
    Ciphertext input_cipher1, input_cipher2, nw_output;
    Ciphertext weight1_cipher, weight2_cipher, weight3_cipher, weight4_cipher, weight_out1_cipher, weight_out2_cipher,
        bias1_cipher, bias2_cipher, bias_out_cipher;

    double scale = pow(2.0, q);
    double bias_scale = pow(2.0, 2 * q);

    time_start = chrono::high_resolution_clock::now();
    encoder.encode(weight1, scale, weight1_plain);
    encoder.encode(weight2, scale, weight2_plain);
    encoder.encode(weight3, scale, weight3_plain);
    encoder.encode(weight4, scale, weight4_plain);
    encoder.encode(weight_out1, scale, weight_out1_plain);
    encoder.encode(weight_out2, scale, weight_out2_plain);
    encoder.encode(bias1, scale, bias1_plain); // -------------- special encoding
    encoder.encode(bias2, scale, bias2_plain);
    encoder.encode(bias_out, bias_scale, bias_out_plain); // -------------------- special encoding
    encoder.encode(x1, scale, input_plain1);
    encoder.encode(x2, scale, input_plain2);
    encryptor.encrypt(input_plain1, input_cipher1);
    encryptor.encrypt(input_plain2, input_cipher2);
    encryptor.encrypt(weight1_plain, weight1_cipher);
    encryptor.encrypt(weight2_plain, weight2_cipher);
    encryptor.encrypt(weight3_plain, weight3_cipher);
    encryptor.encrypt(weight4_plain, weight4_cipher);
    encryptor.encrypt(weight_out1_plain, weight_out1_cipher);
    encryptor.encrypt(weight_out2_plain, weight_out2_cipher);
    encryptor.encrypt(bias1_plain, bias1_cipher);
    encryptor.encrypt(bias2_plain, bias2_cipher);
    encryptor.encrypt(bias_out_plain, bias_out_cipher);
    time_end = chrono::high_resolution_clock::now();
    time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);

    vector<Ciphertext> nw_weights{ weight1_cipher, weight2_cipher,     weight3_cipher,
                                   weight4_cipher, weight_out1_cipher, weight_out2_cipher };
    vector<Ciphertext> nw_bias{ bias1_cipher, bias2_cipher, bias_out_cipher };
    size_t input_per_layer[2] = { 2, 2 };
    size_t output_per_layer[2] = { 2, 1 };
    size_t no_of_layers = 2;

    Ciphertext nw_input[2] = { input_cipher1, input_cipher2 };

    size_t input_size = (input_cipher1.int_array().size()) / 1024 + (input_cipher2.int_array().size()) / 1024 +
                        (weight1_cipher.int_array().size()) / 1024 + (weight2_cipher.int_array().size()) / 1024 +
                        (weight3_cipher.int_array().size()) / 1024 + (weight4_cipher.int_array().size()) / 1024 +
                        (weight_out1_cipher.int_array().size()) / 1024 + (weight_out2_cipher.int_array().size()) / 1024;
    cout << " + Total size of input ciphertexts to the network is : " << input_size << " KB . " << endl;
    cout << " + Total encoding and encryption time of the input ciphertets is : " << time_diff.count()
         << "microseconds ." << endl;

    time_start = chrono::high_resolution_clock::now();
    neural_network(
        context, nw_input, nw_weights, nw_bias, no_of_layers, input_per_layer, output_per_layer, nw_output, q,
        relin_keys);
    time_end = chrono::high_resolution_clock::now();
    time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);

    Plaintext result_plain;
    vector<double> result_vector;
    decryptor.decrypt(nw_output, result_plain);
    encoder.decode(result_plain, result_vector);

    cout << " + Size of the output ciphertext of neural network is : " << (nw_output.int_array().size()) / 1024
         << " KB . " << endl;

    // calculate the accuracy of the evaluated network predicted output
    double j = 0;
    for (int i = 0; i < y.size(); i++)
    {
        if (result_vector[i] >= 0.5)
        {
            y_pred.push_back(1);
        }
        else
        {
            y_pred.push_back(0);
        }

        if (y[i] == y_pred[i])
            j++;
    }

    cout << endl;
    cout << " + Total time for computation of neural network of 2 layers is : " << time_diff.count()
         << " microseconds. " << endl;
    cout << " + Total no. of input samples for non-linear classsification : " << y.size() << endl;
    cout << " + Total no. of correctly classified samples by the network : " << j << endl;
    double accuracy = (j / 105) * 100;
    cout << " + The validation accuracy is  " << accuracy << " percent ." << endl;
    cout << endl;
}
