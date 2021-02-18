#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include "examples.h"
#include "math.h"

using namespace std;
using namespace seal;

std::unique_ptr<seal::PublicKey> pk;
std::unique_ptr<seal::SecretKey> sk;
std::unique_ptr<seal::RelinKeys> rk;

vector<Ciphertext> rotate_right(shared_ptr<SEALContext> context, vector<Ciphertext> x, int64_t s)
{
    Decryptor decryptor(context, *sk);
    IntegerEncoder intencoder(context);

    for (int64_t i = 0; i < s; i++)
    {
        x.insert(x.begin(), x.back());
        x.pop_back();
    }
    cout << " vector after " << s << "  rotations " << endl;
    for (int q = 0; q <= 16; q++)
    {
        Plaintext plain1;
        decryptor.decrypt(x[q], plain1);
        cout << intencoder.decode_int64(plain1);
        cout << endl;
        plain1.release();
    }
    return x;
}


void rotate_mult(shared_ptr<SEALContext> context, Ciphertext input)
{
    Encryptor encryptor(context, *pk);
    Evaluator evaluator(context);
    Decryptor decryptor(context, *sk);
    IntegerEncoder intencoder(context);
    vector<Plaintext> x_plain;
    vector<Ciphertext> x_enc;
    vector<Plaintext> func_indices;
    Ciphertext temp, enc;
    int64_t n = 4;
    int64_t N = pow(2, n);
    int64_t max_input_range = 8;

    func_indices.clear();
    x_enc.clear();
    x_plain.clear();
   
    Plaintext decrp;
    decryptor.decrypt(input, decrp);
    cout << " The input to this function is " << intencoder.decode_int64(decrp) << endl;
    cout << endl;
    decrp.release();

    Plaintext max_input_plain;
    intencoder.encode(max_input_range, max_input_plain);
    evaluator.add_plain(input, max_input_plain, temp);
    max_input_plain.release();

    cout << " Compute indices vector,test input vector and the encrypted x vector" << endl;
    for (int j = 0; j <= N; j++)
    {
        x_enc.push_back(temp);
        func_indices.push_back(intencoder.encode(j));
        evaluator.sub_plain_inplace(x_enc[j], func_indices[j]);
    }

    vector<Ciphertext> rot_x;
    for (size_t s = 0; s < n; s++)
    {
        rot_x.clear();
        rot_x = rotate_right(context, x_enc, pow(2, s));
        Plaintext a1, a2;
        cout << endl;
        cout << " inputs to multiplication " << endl;
        for (int i = 0; i <= N; i++)
        {
            decryptor.decrypt(x_enc[i], a1);
            decryptor.decrypt(rot_x[i], a2);
            cout << intencoder.decode_int64(a1) << " , " << intencoder.decode_int64(a2) << endl;
            evaluator.multiply_inplace(x_enc[i], rot_x[i]);
            evaluator.relinearize_inplace(x_enc[i], *rk);
        }
        Plaintext decd;
        cout << endl;
        cout << " rotation input is " << endl;
        for (int i = 0; i <= N; i++)
        {
            decryptor.decrypt(x_enc[i], decd);
            cout << intencoder.decode_int64(decd) << endl;
        }
        cout << endl;
    }


    cout << " Noise in product x_enc vector is " << endl;
    for (int i = 0; i <= N; i++)
    {
        cout << decryptor.invariant_noise_budget(x_enc[i]) << endl;
        cout << endl;
    }

    x_plain.clear();
    x_enc.clear();
    func_indices.clear();
    temp.release();
    enc.release();
}

int main()
{
    EncryptionParameters parms(scheme_type::BFV);
    size_t poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(12289);
    auto context = SEALContext::Create(parms);
    cout << "Set encryption parameters and print" << endl;
    print_parameters(context);

    KeyGenerator keygen(context);
    pk = std::make_unique<seal::PublicKey>(keygen.public_key());
    sk = std::make_unique<seal::SecretKey>(keygen.secret_key());
    rk = std::make_unique<seal::RelinKeys>(keygen.relin_keys_local());
    Encryptor encryptor(context, *pk);
    Evaluator evaluator(context);
    Decryptor decryptor(context, *sk);
    IntegerEncoder encoder(context);
    
    int64_t v1 = -2;
    int64_t v2 = 3;
    Plaintext v_plain1, v_plain2;
    Ciphertext v_enc1, v_enc2;
    encoder.encode(v1, v_plain1);
    encoder.encode(v2, v_plain2);
    encryptor.encrypt(v_plain2, v_enc2);
    encryptor.encrypt(v_plain1, v_enc1);
    evaluator.multiply_inplace(v_enc1, v_enc2);
    evaluator.relinearize_inplace(v_enc1, *rk);
    rotate_mult(context, v_enc1);
    
    return 0;
}