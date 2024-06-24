#pragma once

// Credit to Marcin Tarsier from the following StackOverflow answer
// https://stackoverflow.com/a/72813010
template<class Specialization, template<typename> class TemplateClass,
         typename ...PartialSpecialisation>
concept specializes = requires (Specialization s) {
    []<typename ...TemplateArgs>(
        TemplateClass<PartialSpecialisation..., TemplateArgs...>&){}(s);
};
