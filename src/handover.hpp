////////////////////////////////////////////////////////////////////////////
//
// Copyright 2016 Realm Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////

#ifndef realm_handover_hpp
#define realm_handover_hpp

#include <realm/row.hpp>
#include <realm/query.hpp>
#include <realm/group_shared.hpp>
#include <realm/link_view.hpp>
#include <realm/table_view.hpp>

namespace realm {
class HandoverPackage;
class SharedGroup;

class AnyHandover;

// Type-erased wrapper for any type which must be exported to be handed between threads
class AnyThreadConfined {
public:
    enum class Type {
        Row,
        Query,
        TableRef,
        TableView,
        LinkViewRef,
    };
    
    // Constructors
    AnyThreadConfined(Row row)                   : m_type(Type::Row),         m_row(row)                     { }
    AnyThreadConfined(Query query)               : m_type(Type::Query),       m_query(query)                 { }
    AnyThreadConfined(TableRef table_ref)        : m_type(Type::TableRef),    m_table_ref(table_ref)         { }
    AnyThreadConfined(TableView table_view)      : m_type(Type::TableView),   m_table_view(table_view)       { }
    AnyThreadConfined(LinkViewRef link_view_ref) : m_type(Type::LinkViewRef), m_link_view_ref(link_view_ref) { }

    AnyThreadConfined(const AnyThreadConfined& thread_confined);
    AnyThreadConfined(AnyThreadConfined&& thread_confined);
    ~AnyThreadConfined();

    Type type() const { return m_type; }

    // Getters
    Row         get_row()           const { REALM_ASSERT(m_type == Type::Row);         return m_row;           }
    Query       get_query()         const { REALM_ASSERT(m_type == Type::Query);       return m_query;         }
    TableRef    get_table_ref()     const { REALM_ASSERT(m_type == Type::TableRef);    return m_table_ref;     }
    TableView   get_table_view()    const { REALM_ASSERT(m_type == Type::TableView);   return m_table_view;    }
    LinkViewRef get_link_view_ref() const { REALM_ASSERT(m_type == Type::LinkViewRef); return m_link_view_ref; }

    AnyHandover export_for_handover(SharedGroup &shared_group) const;

private:
    Type m_type;
    union {
        Row m_row;
        Query m_query;
        TableRef m_table_ref;
        TableView m_table_view;
        LinkViewRef m_link_view_ref;
    };
};

// Type-erased wrapper for a `Handover` of an `AnyThreadConfined` value
class AnyHandover {
private:
    friend AnyHandover AnyThreadConfined::export_for_handover(SharedGroup &shared_group) const;

    enum class Type {
        Row,
        Query,
        Table,
        TableView,
        LinkView,
    };

    using RowHandover       = std::unique_ptr<SharedGroup::Handover<Row>>;
    using QueryHandover     = std::unique_ptr<SharedGroup::Handover<Query>>;
    using TableHandover     = std::unique_ptr<SharedGroup::Handover<Table>>;
    using TableViewHandover = std::unique_ptr<SharedGroup::Handover<TableView>>;
    using LinkViewHandover  = std::unique_ptr<SharedGroup::Handover<LinkView>>;

    Type m_type;
    union {
        RowHandover m_row;
        QueryHandover m_query;
        TableHandover m_table;
        TableViewHandover m_table_view;
        LinkViewHandover m_link_view;
    };

    // Constructors
    AnyHandover(RowHandover row)              : m_type(Type::Row),       m_row(std::move(row))               { }
    AnyHandover(QueryHandover query)          : m_type(Type::Query),     m_query(std::move(query))           { }
    AnyHandover(TableHandover table)          : m_type(Type::Table),     m_table(std::move(table))           { }
    AnyHandover(TableViewHandover table_view) : m_type(Type::TableView), m_table_view(std::move(table_view)) { }
    AnyHandover(LinkViewHandover link_view)   : m_type(Type::LinkView),  m_link_view(std::move(link_view))   { }

public:
    AnyHandover(AnyHandover&& handover);
    ~AnyHandover();

    AnyThreadConfined import_from_handover(SharedGroup &shared_group) &&;
};
}

#endif /* realm_handover_hpp */
