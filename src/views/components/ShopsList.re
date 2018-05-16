let component = ReasonReact.statelessComponent("shops");

open Types;

let make = (~data: list(shop), _children) => {
  ...component,
  render: _self =>
    data
    |> Array.of_list
    |> Array.map(shop =>
         <div key=shop.id className="shop-wrapper">
           <p> (ReasonReact.string(shop.name)) </p>
           <img src=shop.picture />
         </div>
       )
    |> ReasonReact.array
};