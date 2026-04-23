-- 新建数据库
DROP DATABASE IF EXISTS topic01;
CREATE DATABASE topic01 CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci;
use topic01;

-- 班级表
drop table if exists class;
create table class (
  id bigint primary key auto_increment,
  name varchar(20)
);

-- 学生表
drop table if exists student;
create table student (
  id bigint primary key auto_increment,
  name varchar(20) not null, 
  sno varchar(10) not null,
  age int default 18,
  gender tinyint(1), 
  enroll_date date,
  class_id bigint,
  foreign key (class_id) references class(id)
);

-- 课程表
drop table if exists course;
create table course (
  id bigint primary key auto_increment,
  name varchar(20)
);

-- 分数表
drop table if exists score;
create table score (
  id bigint primary key auto_increment,
  score float,
  student_id bigint,
  course_id bigint,
  foreign key (student_id) references student(id),
  foreign key (course_id) references course(id)
);

-- 课程表
insert into course (name) values ('Java'), ('C++'), ('MySQL'), ('操作系统'), ('计算机网络'), ('数据结构');

-- 班级表
insert into class(name) values ('Java001班'), ('C++001班'), ('前端001班');

-- 学生表
insert into student (name, sno, age, gender, enroll_date, class_id) values 
('唐三藏', '100001', 18, 1, '1986-09-01', 1),
('孙悟空', '100002', 18, 1, '1986-09-01', 1),
('猪悟能', '100003', 18, 1, '1986-09-01', 1),
('沙悟净', '100004', 18, 1, '1986-09-01', 1),
('宋江', '200001', 18, 1, '2000-09-01', 2),
('武松', '200002', 18, 1, '2000-09-01', 2),
('李逹', '200003', 18, 1, '2000-09-01', 2),
('不想毕业', '200004', 18, 1, '2000-09-01', 2);

-- 成绩表
insert into score (score, student_id, course_id) values
(70.5, 1, 1),(98.5, 1, 3),(33, 1, 5),(98, 1, 6),
(60, 2, 1),(59.5, 2, 5),
(33, 3, 1),(68, 3, 3),(99, 3, 5),
(67, 4, 1),(23, 4, 3),(56, 4, 5),(72, 4, 6),
(81, 5, 1),(37, 5, 5),
(56, 6, 2),(43, 6, 4),(79, 6, 6),
(80, 7, 2),(92, 7, 6);


-- 创建考试成绩表练习表
DROP TABLE IF EXISTS exam;
CREATE TABLE exam (
id bigint,
name VARCHAR(20),
chinese DECIMAL(4,1),
math DECIMAL(4,1),
english DECIMAL(4,1)
);
-- 插入测试数据
INSERT INTO exam (id,name, chinese, math, english) VALUES
(1,'唐三藏', 67, 98, 56),
(2,'孙悟空', 87.5, 78, 77),
(3,'猪悟能', 88, 98, 90),
(4,'曹孟德', 82, 84, 67),
(5,'刘玄德', 55.5, 85, 45),
(6,'孙权', 70, 73, 78.5),
(7,'宋公明', 75, 65, 30);