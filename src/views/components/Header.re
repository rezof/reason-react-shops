[%bs.raw "require('../../styles/header.css')"];

open Types;

let component = ReasonReact.statelessComponent("Header");

let make = (~tabs: list(tab), ~selectedTab, ~changeSelectedTab, _children) => {
  ...component,
  render: _self =>
    <div className="header">
      <div className="menu-items">
        (
          tabs
          |> Array.of_list
          |> Array.mapi((i, tab) => {
               let selected = tab.id === selectedTab ? "selected" : "";
               let clickHandler =
                 tab.id !== selected ? changeSelectedTab(tab.id) : (_e => ());
               <span
                 key=(string_of_int(i))
                 onClick=clickHandler
                 className=("menu-item " ++ selected)>
                 (ReasonReact.string(tab.text))
               </span>;
             })
          |> ReasonReact.array
        )
      </div>
      <div className="logout-wrapper">
        <a href="#"> (ReasonReact.string("Logout")) </a>
      </div>
    </div>
};