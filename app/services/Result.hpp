#pragma once

namespace result {
    template<
        class Ok,
        class Err
    >
    class result {
    private:
        bool state;

    public:
        Ok  ok;
        Err err;

        result(Ok  o) : ok(o), err( ), state(true)  { }
        result(Err e) : ok( ), err(e), state(false) { }

        bool operator!    () { return !state; }
             operator bool() { return  state; }
    };
}
