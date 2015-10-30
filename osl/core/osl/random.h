#ifndef OSL_RANDOM_H
#define OSL_RANDOM_H
namespace osl
{
  namespace misc
  {
    unsigned int random();
    unsigned int time_seeded_random();
    template<typename T>
    struct Random;
    template<>
    struct Random<unsigned int>{
      static unsigned int newValue(){
	return random();
      }
    };
    template<>
    struct Random<unsigned long long>{
      static unsigned long long newValue(){
	return (static_cast<unsigned long long>(random())<<32ull)|
	  static_cast<unsigned long long>(random());
      }
    };
  } // namespace misc
  using osl::misc::random;
  using osl::misc::time_seeded_random;
} // namespace osl
#endif /* _RANDOM_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
