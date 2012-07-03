create table student (
	id integer,
	name char(64)
);
create table lecture (
	id integer,
	name char(64)
);
create table exam (
	l_id integer,
	s_id integer,
	grade integer
);
