create table if not exists estoque_items (
    id bigserial primary key,
    code varchar(50) not null unique,
    item_type varchar(20) not null check (item_type in ('product', 'material')),
    name varchar(255) not null,
    description text not null,
    information_link text not null,
    quantity integer not null check (quantity >= 0),
    unit_price double precision not null check (unit_price >= 0),
    location varchar(255) not null,
    created_at timestamptz not null default now(),
    updated_at timestamptz not null default now()
);

create table if not exists estoque_inventory_movements (
    id bigserial primary key,
    item_id bigint not null references estoque_items(id) on delete cascade,
    operation varchar(20) not null check (operation in ('inbound', 'outbound')),
    quantity integer not null check (quantity > 0),
    note text not null,
    created_at timestamptz not null default now()
);

create index if not exists idx_estoque_items_name_lower on estoque_items (lower(name));
create index if not exists idx_estoque_inventory_movements_item_id on estoque_inventory_movements (item_id);
