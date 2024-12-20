/*
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef EPROSIMA_XTYPES_SEQUENCE_TYPE_HPP_
#define EPROSIMA_XTYPES_SEQUENCE_TYPE_HPP_

#include <xtypes/MutableCollectionType.hpp>
#include <xtypes/SequenceInstance.hpp>
#include <xtypes/StructType.hpp>

#include <vector>

namespace eprosima {
namespace xtypes {

/// \brief DynamicType representing mutable sequence of elements.
/// A SequenceType represents a TypeKind::SEQUENCE_TYPE.
class SequenceType : public MutableCollectionType
{
public:

    /// \brief Construct a SequenceType.
    /// \param[in] content Content type of the sequence.
    /// \param[in] bounds Size limit of the sequence, 0 means that no limits.
    explicit SequenceType(
            const DynamicType& content,
            uint32_t bounds = 0)
        : MutableCollectionType(
            TypeKind::SEQUENCE_TYPE,
            "sequence_" + ((bounds > 0) ? std::to_string(bounds) + "_" : "") + content.name(),
            DynamicType::Ptr(content),
            bounds)
    {
    }

    /// \brief Construct a SequenceType.
    /// \param[in] content Content type of the sequence.
    /// \param[in] bounds Size limit of the sequence, 0 means that no limits.
    template<typename DynamicTypeImpl>
    SequenceType(
            const DynamicTypeImpl&& content,
            uint32_t bounds)
        : MutableCollectionType(
            TypeKind::SEQUENCE_TYPE,
            "sequence_" + ((bounds > 0) ? std::to_string(bounds) + "_" : "") + content.name(),
            DynamicType::Ptr(std::move(content)),
            bounds)
    {
    }

    SequenceType(
            const SequenceType& other) = default;
    SequenceType(
            SequenceType&& other) = default;

    virtual size_t memory_size() const override
    {
        return sizeof(SequenceInstance);
    }

    virtual void construct_instance(
            uint8_t* instance) const override
    {
        new (instance) SequenceInstance(content_type(), bounds());
    }

    virtual void copy_instance(
            uint8_t* target,
            const uint8_t* source) const override
    {
        new (target) SequenceInstance(*reinterpret_cast<const SequenceInstance*>(source));
    }

    virtual void copy_instance_from_type(
            uint8_t* target,
            const uint8_t* source,
            const DynamicType& arg_other) const override
    {
        const DynamicType& other = (arg_other.kind() == TypeKind::ALIAS_TYPE)
                ? static_cast<const AliasType&>(arg_other).rget()
                : arg_other;

        if (other.kind() == TypeKind::STRUCTURE_TYPE)
        {
            // Resolve one-member struct compatibility
            const StructType& struct_type = static_cast<const StructType&>(other);
            if (struct_type.members().size() == 1)
            {
                copy_instance_from_type(target, source, struct_type.members().at(0).type());
                return;
            }
        }

        xtypes_assert(other.kind() == TypeKind::SEQUENCE_TYPE,
                "Cannot copy data from different types: From '" << other.name() << "' to '" << name() << "'.");

        (void) other;
        new (target) SequenceInstance(*reinterpret_cast<const SequenceInstance*>(source), content_type(), bounds());
    }

    virtual void move_instance(
            uint8_t* target,
            uint8_t* source,
            bool initialized) const override
    {
        if (initialized)
        {
            destroy_instance(target);
        }
        new (target) SequenceInstance(std::move(*reinterpret_cast<const SequenceInstance*>(source)));
    }

    virtual void destroy_instance(
            uint8_t* instance) const override
    {
        reinterpret_cast<SequenceInstance*>(instance)->~SequenceInstance();
    }

    virtual uint8_t* get_instance_at(
            uint8_t* instance,
            size_t index) const override
    {
        return reinterpret_cast<SequenceInstance*>(instance)->operator [](uint32_t(index));
    }

    virtual size_t get_instance_size(
            const uint8_t* instance) const override
    {
        return reinterpret_cast<const SequenceInstance*>(instance)->size();
    }

    virtual bool compare_instance(
            const uint8_t* instance,
            const uint8_t* other_instance) const override
    {
        return *reinterpret_cast<const SequenceInstance*>(instance)
               == *reinterpret_cast<const SequenceInstance*>(other_instance);
    }

    virtual TypeConsistency is_compatible(
            const DynamicType& other) const override
    {
        if (other.kind() == TypeKind::ALIAS_TYPE)
        {
            const AliasType& other_alias = static_cast<const AliasType&>(other);
            return is_compatible(other_alias.rget());
        }

        if (other.kind() == TypeKind::STRUCTURE_TYPE) // Resolve one-member structs
        {
            return other.is_compatible(*this);
        }

        if (other.kind() != TypeKind::SEQUENCE_TYPE)
        {
            return TypeConsistency::NONE;
        }

        const SequenceType& other_sequence = static_cast<const SequenceType&>(other);

        if (bounds() == other_sequence.bounds())
        {
            return TypeConsistency::EQUALS
                   | content_type().is_compatible(other_sequence.content_type());
        }

        return TypeConsistency::IGNORE_SEQUENCE_BOUNDS
               | content_type().is_compatible(other_sequence.content_type());
    }

    virtual void for_each_instance(
            const InstanceNode& node,
            InstanceVisitor visitor) const override
    {
        const SequenceInstance& sequence = *reinterpret_cast<const SequenceInstance*>(node.instance);
        visitor(node);
        for (uint32_t i = 0; i < sequence.size(); i++)
        {
            InstanceNode child(node, content_type(), sequence[i], i, nullptr);
            content_type().for_each_instance(child, visitor);
        }
    }

    virtual void for_each_type(
            const TypeNode& node,
            TypeVisitor visitor,
            bool preorder=true) const override
    {
        if(preorder)
        {
            visitor(node);
        }
        TypeNode child(node, content_type(), 0, nullptr);
        content_type().for_each_type(child, visitor, preorder);
        if(!preorder)
        {
            visitor(node);
        }
    }

    /// \brief Push a value to a sequence instance.
    /// \param[in, out] instance Memory instance representing a SequenceInstance.
    /// \param[in] value to add into the sequence.
    /// \returns a instance location representing the new value added
    /// or nullptr if the sequence reach the limit.
    uint8_t* push_instance(
            uint8_t* instance,
            const uint8_t* value) const
    {
        if (get_instance_size(instance) < bounds() || bounds() == 0)
        {
            return reinterpret_cast<SequenceInstance*>(instance)->push(value, bounds());
        }
        return nullptr;
    }

    /// \brief Resize a sequence instance to reach the requested size.
    /// All new values needed will be default-initialized
    /// \param[in, out] instance Memory instance representing a SequenceInstance.
    /// \param[in] size new sequence instance size.
    void resize_instance(
            uint8_t* instance,
            size_t size) const
    {
        reinterpret_cast<SequenceInstance*>(instance)->resize(size, bounds());
    }

    virtual uint64_t hash(
            const uint8_t* c_instance) const override
    {
        if (bounds() > 0)
        {
            uint8_t* instance = const_cast<uint8_t*>(c_instance);
            if (content_type().is_constructed_type())
            {
                uint64_t h = content_type().hash(instance);
                for (size_t i = 1; i < bounds(); ++i)
                {
                    Instanceable::hash_combine(h, content_type().hash(get_instance_at(instance, i)));
                }
                return h;
            }
            else
            {
                return Instanceable::hash(instance);
            }
        }
        return 0;
    }

protected:

    std::shared_ptr<DynamicType> clone() const override
    {
        return std::make_shared<SequenceType>(*this);
    }

};

} //namespace xtypes
} //namespace eprosima

#endif //EPROSIMA_XTYPES_SEQUENCE_TYPE_HPP_
