use test;

drop table if exists one;

create table one(
    field1 varchar(10),
    field2 varchar(20),
    field3 timestamp default current_timestamp);

insert into one (field1, field2) values('test1', 'test');
insert into one (field1, field2) values('test2', 'test');
insert into one (field1, field2) values('test3', 'test');
insert into one (field1, field2) values('test4', 'test');
insert into one (field1, field2) values('test5', 'test');
insert into one (field1, field2) values('test6', 'test');
insert into one (field1, field2) values('test7', 'test');
insert into one (field1, field2) values('test8', 'test');
insert into one (field1, field2) values('test9', 'test');
