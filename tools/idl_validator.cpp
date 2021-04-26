#include <xtypes/xtypes.hpp>
#include <xtypes/idl/idl.hpp>

using namespace eprosima::xtypes;

int main(
        int argc,
        char** argv)
{
    if (argc != 2)
    {
        std::cout << "The IDL must be passed as argument" << std::endl;
        return -1;
    }
    else
    {
        std::string idl_spec = argv[1];
        std::cout << "IDL: " << idl_spec << std::endl;
        idl::Context context;
        context.log_level(idl::log::LogLevel::DEBUG);
        context.print_log(true);
        context.ignore_redefinition = true;
        context = idl::parse(idl_spec, context);
        std::cout << "Parse Success: " << std::boolalpha << context.success << std::endl;
        for (auto [name, type] : context.get_all_types())
        {
            if (type->kind() == TypeKind::STRUCTURE_TYPE)
            {
                std::cout << "Struct Name:" << name << std::endl;
                auto members = static_cast<const StructType*>(type.get())->members();
                for (auto &m: members)
                {
                    std::cout << "Struct Member:" << name << "[" << m.name() << "," << m.type().name() << "]" << std::endl;
                }
            }
        }
        return context.success ? 0 : 1;
    }
}
