#pragma once
// Just a few useful functions  kept here for readability.

// Get a string from a number of any type. boost::lexical_cast<T>() is better...
template<class T> inline
std::string t_to_string(T i){
    std::stringstream ss;
    std::string s;
    ss << i;
    s = ss.str();

    return s;
}

// define some useful functions
int GetWindowString(HWND hwnd, std::string &s);
void SetWindowString(HWND hwnd, std::string s);
int isPowerOfTwo (unsigned int x);

