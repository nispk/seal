#include "examples.h"
#include "math.h"
#include <iostream>

using namespace std;
using namespace seal;

void sigmoid(shared_ptr<SEALContext> context, Ciphertext &input)
{
    auto &context_data = *context->get_context_data(input.parms_id());
    auto &parms = context_data.parms();
    auto &coeff_modulus = parms.coeff_modulus();
    double scale = pow(2.0, 30);
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

    cout << input.data() << endl;
    Plaintext plain;
    vector<double> vec;
    decryptor.decrypt(input,plain);
    encoder.decode(plain, vec);
    cout << " The input obtained  " << endl; // the decrypted output is different from the decrypted result in main program.
    print_vector(vec, 3, 8);
}

int main()
{
    EncryptionParameters parms(scheme_type::CKKS);
    size_t poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(
        CoeffModulus::Create(poly_modulus_degree, { 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
                                                    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 }));

    double scale = pow(2.0, 30);
    auto context = SEALContext::Create(parms);
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
    cout << "Number of slots: " << slot_count << endl;

    double input = 0.769366402;
    double weight = -0.406239561;
    double bias = -9.158888066;

    Plaintext input_plain, weight_plain, bias_plain;
    Ciphertext input_cipher, weight_cipher, bias_cipher, node_output, temp1;

    encoder.encode(input, scale, input_plain);
    encoder.encode(weight, scale, weight_plain);
    encryptor.encrypt(input_plain, input_cipher);
    encryptor.encrypt(weight_plain, weight_cipher);
    evaluator.multiply(input_cipher, weight_cipher, temp1);
    evaluator.relinearize_inplace(temp1, relin_keys);
    evaluator.rescale_to_next_inplace(temp1);

    encoder.encode(bias, temp1.scale(), bias_plain);
    encryptor.encrypt(bias_plain, bias_cipher);
    evaluator.mod_switch_to_inplace(bias_cipher, temp1.parms_id());
    cout << " bias scale after mod switch " << log2(bias_cipher.scale()) << endl;
    evaluator.add(temp1, bias_cipher, node_output);

    cout << node_output.data();
    cout << endl;

    Plaintext pl;
    vector<double> vect;
    decryptor.decrypt(node_output, pl);
    encoder.decode(pl, vect);
    cout << " The input sent to the function " << endl;
    print_vector(vect, 3, 8);
    cout << endl;
    sigmoid(context, node_output);

    return 0;
}